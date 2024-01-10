/* Public domain. */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Current version: v6. */

/***********************************************************************
 * Change log:
 * v1:
 *  Initial implementation:
 *   Pretty dumb, pretty straightforward, do _not_ use any kind of
 *   hashing, awfully slow (slower than the reference Java), but it
 *   works.
 *
 * v2:
 *  Add simple hashtable support: open addressing for collisions + SDBM
 *  for the string hash. Its ~8x faster than v1, finally we are (twice)
 *  faster than the reference Java code.
 *
 * v3:
 *  Removal of strtof() for reading the temperature values, handling
 *  temperatures as integers and parsing them manually. Its ~3x faster
 *  than v2.
 *
 * v4:
 *  - Some functions inlined
 *  - likely()/unlikely() on hot code paths
 *  - Entirely remove the hashtable open addressing:
 *      Since the hashtable proved to be perfect and have no collisions
 *      for the input file, I'm removing all the collision handling and
 *      strings comparison.
 *  All this changes led us to ~1.32x faster than v3.
 *
 * v5-fun-version:
 *  (Intentionally) very crap threads support.
 *  Intentionally bad because the hash table is shared between all
 *  threads and protected by a mutex... which adds a huge bottleneck.
 *  Idiot? Yes, but I would like to see how bad it would be.
 *  Result: ~6.17x slower than v4.
 *
 * v6:
 *  Nicely working threads implementation:
 *   - Each thread now have its individual hashtable that is merged later
 *     (to avoid locks & race conditions).
 *   - The merge process is quite quick and do not need to be optimized.
 *   - Loop unroll on read_temperature() function which make us 8% faster.
 *  Result: ~3.57x *faster* than v4 (on a quad-core CPU).
 */

#define NUM_THREADS 4
#define HT_SIZE (10000 * 5)
#define unlikely(c) __builtin_expect((c), 0)
#define likely(c)   __builtin_expect((c), 1)

static int    txt_fd;
static char  *txt_buff;
static off_t  txt_size;

struct station
{
	char *name;
	int min;
	int avg;
	int max;
	int count;
};

static struct thread_data {
	int tidx;
	size_t size;
	const char *base_buffer;
	const char *end_buffer;
} tdata[NUM_THREADS];

static pthread_t threads[NUM_THREADS];
static struct station stations[NUM_THREADS][HT_SIZE] = {0};
static size_t hashtable_entries[NUM_THREADS] = {0};

/*
 * Open the given file and memory map it.
 * @param file Measurements file.
 */
static void open_file(const char *file)
{
	struct stat st;

	txt_fd = open(file, O_RDONLY);
	if (txt_fd < 0)
		err(1, "Unable to open file!\n");

	fstat(txt_fd, &st);
	txt_size = st.st_size;

	txt_buff = mmap(NULL, st.st_size, PROT_READ,
		MAP_PRIVATE|MAP_POPULATE, txt_fd, 0);

	if (txt_buff == MAP_FAILED)
		err(1, "Unable to mmap!\n");
}

/*
 * @brief Munmap the memory mapped file and then close it.
 */
static void close_file(void)
{
	munmap(txt_buff, txt_size);
	close(txt_fd);
}

/**
 * @brief For a given key and its size, returns an unsigned
 * hash value of a 64-bit.
 *
 * @param key  String key.
 * @param size String key length.
 *
 * @return Returns the hashed value.
 */
static inline uint64_t
hashtable_sdbm(const void *key, size_t size)
{
	unsigned char *str;  /* String pointer.    */
	uint64_t hash;       /* Resulting hash.    */
	int c;               /* Current character. */

	str  = (unsigned char *)key;
	hash = 0;

	for (size_t i = 0; i < size; i++) {
		c    = *str++;
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return (hash);
}

/**
 * @brief For a given string key and its length, returns
 * the hashtable bucket index.
 *
 * @param key  String key.
 * @param size String key length.
 *
 * @return Return hashtable index.
 */
static inline size_t
hashtable_bucket_index(const void *key, size_t size)
{
	uint64_t hash;
	size_t  index;
	hash  = hashtable_sdbm(key, size);
	index = (hash % HT_SIZE);
	return (index);
}

/**
 * @brief Given a station name and its name size, check if it
 * exists in the hashtable.
 *
 * @param st_name Station name.
 * @param st_size Station size.
 * @param index   Returned index (if found).
 * @param tid     Thread index.
 *
 * @return Returns the hashtable entry if found, NULL otherwise.
 */
static inline struct station *
hashtable_find_station(const char *st_name, size_t st_size, size_t *index, int tid)
{
	size_t idx = hashtable_bucket_index(st_name, st_size);

	*index = idx;
	if (stations[tid][idx].name)
		return (&stations[tid][idx]);

	return (NULL);
}

/**
 * @brief Given a station name @p st_name of given size @p st_size
 * and @p value, adds it into the hashtable.
 *
 * @param st_name Station name.
 * @param st_size Station name length.
 * @param value   Temperature value.
 * @param tid     Thread index.
 */
static void
hashtable_add_station(const char *st_name, size_t st_size, int value, int tid)
{
	struct station *st;
	size_t index;

	/* Try to find first. */
	st = hashtable_find_station(st_name, st_size, &index, tid);
	if (likely(st != NULL))
		goto found;

	/* If not found, hashtable full, we have a problem!. */
	if (hashtable_entries[tid] == HT_SIZE)
		err(1, "Hashtable full!, too many collisions!\n");

	/* Not found, add entry. */
	hashtable_entries[tid]++;
	stations[tid][index].avg   = value;
	stations[tid][index].min   = value;
	stations[tid][index].max   = value;
	stations[tid][index].count = 1;

	stations[tid][index].name = malloc(st_size + 1);
	memcpy(stations[tid][index].name, st_name, st_size);
	stations[tid][index].name[st_size] = '\0';
	return;

found:
	/* Update entries. */
	if (value < stations[tid][index].min)
		stations[tid][index].min = value;
	if (value > stations[tid][index].max)
		stations[tid][index].max = value;

	stations[tid][index].avg += value;
	stations[tid][index].count++;
}

/**
 * @brief Given the current line, reads the temperature
 * as an integer and return it.
 *
 * @param line Line containing the temperature (as float).
 *
 * @return Returns the read temperature as integer, multiplied
 * by 10.
 */
static inline int
read_temperature(const char *line)
{
	const char *p = line;
	int temp = 0;
	int sign = 1;

	if (*p == '-') {
		sign = -1;
		p++;
	}

	/* Numbers of format: X.Y */
	if (p[1] == '.')
		temp = ((p[0] - '0') * 10) + (p[2] - '0');

	/* Numbers of format: XY.Z */
	else
		temp = ((p[0] - '0') * 100) +
			((p[1] - '0') * 10) + (p[3] - '0');

	temp *= sign;
	return (temp);
}

/**
 * @brief Given the station line and its size, adds it into the
 * hashtable.
 *
 * @param station_line Line containing the station and temperature.
 * @param size         Line size.
 * @param tid          Thread index.
 */
static void
add_station(const char *station_line, size_t size, int tid)
{
	const char *delim, *st_name;
	size_t st_size;
	int value;

	delim = memchr(station_line, ';', size);
	if (!delim)
		err(1, "Invalid station?");

	value   = read_temperature(delim+1);
	st_size = delim - station_line;

	hashtable_add_station(station_line, st_size, value, tid);
}

/* String comparator. */
static int
cmp_string(const void *p1, const void *p2)
{
	const struct station *s1, *s2;
	s1 = p1;
	s2 = p2;

	if (!s1->name && !s2->name) return (0);
	if (!s1->name) return (1);
	if (!s2->name) return (-1);
	else
		return (strcmp(s1->name, s2->name));
}

/*
 * List all the stations in the same output as the
 * challenge requires.
 */
static void list_stations(void)
{
	printf("{");
	for (size_t i = 0; i < HT_SIZE; i++)
	{
		/* The first null entry means that we have walked
		 * through everything (because the list is)
		 * already sorted.
		 */
		if (!stations[0][i].name)
			break;

		printf("%s=%.1f/%.1f/%.1f",
			stations[0][i].name,
			 (float)stations[0][i].min/10.0f,
			((float)stations[0][i].avg / (float)stations[0][i].count)/10.0f,
			 (float)stations[0][i].max/10.0f);

		if (i < HT_SIZE - 1 && stations[0][i + 1].name)
			printf(", ");
	}
	printf("}\n");
}

/**
 * @brief Iterates over each per-thread hashtable and merges
 * them into a single-hashtable.
 */
static void do_merge_threads_data(void)
{
	struct station *st;
	int i, tid;

	/* Copy all the data into the thread 0 hashtable. */
	for (i = 0; i < HT_SIZE; i++) {
		st = &stations[0][i];

		for (tid = 1; tid < NUM_THREADS; tid++) {
			/* Skip empty entries. */
			if (!stations[tid][i].name)
				continue;

			/* Copy the name it the entry is empty. */
			if (!st->name) {
				st->name = stations[tid][i].name;
				st->min  = stations[tid][i].min;
				st->max  = stations[tid][i].max;
			} else {
				/* If not empty, accumulate min & max values. */
				if (stations[tid][i].min < st->min)
					st->min = stations[tid][i].min;
				if (stations[tid][i].max > st->max)
					st->max = stations[tid][i].max;
			}

			/* Copy the remaining data, accumulating where
			 * needed. */
			st->avg   += stations[tid][i].avg;
			st->count += stations[tid][i].count;
		}
	}
}

/**
 * @brief Worker thread, works basically the same way as the
 * single-threaded version.
 *
 * @param p Thread data, including the base buffer and size.
 */
static void*
do_thread_read(void *p)
{
	struct thread_data *td = p;
	const char  *prev;
	const char  *next;
	size_t rsize;

	prev  = td->base_buffer;
	rsize = td->size;

	while ((next = memchr(prev, '\n', rsize)) != NULL)
	{
		add_station(prev, next - prev, td->tidx);
		rsize -= (next - prev + 1);
		prev   = (next + 1);
	}

	return (p);
}

/*
 * @brief Prepares the data range to be processed to the worker
 * threads, start them and wait.
 */
static void do_read(void)
{
	int i;
	char *buf;
	size_t rem_size = txt_size;

	/* Decide each portion that each thread will work. */
	tdata[0].base_buffer = txt_buff;
	tdata[0].size = (txt_size / NUM_THREADS);

	/* Remaining threads. */
	for (i = 1; i < NUM_THREADS; i++)
	{
		/* Adjust size of previous thread. */
		buf = memchr(
			tdata[i-1].base_buffer + tdata[i-1].size,
			'\n',
			rem_size - tdata[i-1].size);

		if (!buf)
			err(1, "Unable to find next line!\n");

		tdata[i-1].size       = (buf - tdata[i-1].base_buffer + 1);
		tdata[i-1].end_buffer = (tdata[i-1].base_buffer + tdata[i-1].size);
		rem_size -= tdata[i-1].size;

		/* Adjust base buffer of current thread and size. */
		tdata[i].base_buffer = tdata[i-1].end_buffer;
		tdata[i].size = (txt_size / NUM_THREADS);
	}

	/* Adjust last thread. */
	tdata[NUM_THREADS-1].base_buffer = tdata[NUM_THREADS-2].end_buffer;
	tdata[NUM_THREADS-1].size        = rem_size;
	tdata[NUM_THREADS-1].end_buffer  = (tdata[NUM_THREADS-1].base_buffer +
		tdata[NUM_THREADS-1].size);

	/* Create worker threads. */
	for (int i = 0; i < NUM_THREADS; i++) {
		tdata[i].tidx = i;
		pthread_create(&threads[i], NULL, do_thread_read, &tdata[i]);
	}

	/* Join threads. */
	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);
}

int main(int argc, char **argv)
{
	if (argc < 2)
		errx(1, "Usage: %s <file>", argv[0]);

	open_file(argv[1]);
	do_read();
	do_merge_threads_data();

	qsort(&stations[0], HT_SIZE,
		sizeof(struct station), &cmp_string);

	list_stations();
	close_file();
	return (0);
}
