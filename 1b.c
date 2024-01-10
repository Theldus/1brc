/* Public domain. */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Current version: v2. */

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
 */

#define HT_SIZE (10000 * 5)

static int    txt_fd;
static char  *txt_buff;
static off_t  txt_size;

struct station
{
	char *name;
	float min;
	float avg;
	float max;
	int count;
};

static struct station stations[HT_SIZE] = {0};
static size_t hashtable_entries = 0;

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
static uint64_t
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
 *
 * @return Returns the hashtable entry if found, NULL otherwise.
 */
static struct station *
hashtable_find_station(const char *st_name, size_t st_size, size_t *index)
{
	size_t idx = hashtable_bucket_index(st_name, st_size);
	size_t i,j;

	if (stations[idx].name) {
		if (!strncmp(st_name, stations[idx].name, st_size)) {
			*index = idx;
			return (&stations[idx]);
		}
		else
			idx++;
	}

	/* Do open addressing to try to find the entry. */
	idx %= HT_SIZE;
	*index = idx;

	for (i = 0, j = idx; i < HT_SIZE; i++, j = (j+1)%HT_SIZE) {
		if (stations[j].name) {
			if (!strncmp(st_name, stations[j].name, st_size)) {
				*index = j;
				return (&stations[j]);
			}
		}
	}
	return (NULL);
}

/**
 * @brief Given a station name @p st_name of given size @p st_size
 * and @p value, adds it into the hashtable.
 */
static void
hashtable_add_station(const char *st_name, size_t st_size, float value)
{
	struct station *st;
	size_t index;
	size_t i,j;

	i = j = 0;

	/* Try to find first. */
	st = hashtable_find_station(st_name, st_size, &index);
	if (st)
		goto found;

	/* Not found... I'm lazy, so open addressing. */
	for (size_t i = 0, j = index; i < HT_SIZE; i++, j=(j+1)%HT_SIZE) {
		if (!stations[j].name) {
			index = j;
			goto add_entry;
		}
	}

	/* If not found, hashtable full, we have a problem!. */
	if (hashtable_entries == HT_SIZE)
		err(1, "Hashtable full!, too many collisions!\n");

	/* Not found. */
add_entry:
	hashtable_entries++;
	if (j != 0 && j != index)
		fprintf(stderr, "Open addressing, idx: %zu, j: %zu!!\n", index, j);

	stations[index].avg   = value;
	stations[index].min   = value;
	stations[index].max   = value;
	stations[index].count = 1;

	stations[index].name = malloc(st_size + 1);
	memcpy(stations[index].name, st_name, st_size);
	stations[index].name[st_size] = '\0';
	return;

found:
	/* Update entries. */
	if (value < stations[index].min)
		stations[index].min = value;
	if (value > stations[index].max)
		stations[index].max = value;

	stations[index].avg += value;
	stations[index].count++;
}

/**
 * @brief Given the station line and its size, adds it into the
 * hashtable.
 * @param station_line Line containing the station and temperature.
 * @param size Line size.
 */
static void add_station(const char *station_line, size_t size)
{
	const char *delim, *st_name;
	size_t st_size;
	char *endptr;
	float value;

	delim = memchr(station_line, ';', size);
	if (!delim)
		err(1, "Invalid station?");

	value   = strtof(delim+1, &endptr);
	st_size = delim - station_line;

	hashtable_add_station(station_line, st_size, value);
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
	for (size_t i = 0; i < hashtable_entries; i++)
	{
		if (!stations[i].name)
			continue;

		printf("%s=%.1f/%.1f/%.1f",
			stations[i].name,
			stations[i].min,
			stations[i].avg / (float)stations[i].count,
			stations[i].max);

		if (i < hashtable_entries - 1)
			printf(", ");
	}
	printf("}\n");
}

/*
 * Read each line and add the station.
 */
static void do_read(void)
{
	char  *prev = txt_buff;
	char  *next = NULL;
	size_t size = txt_size;

	while ((next = memchr(prev, '\n', size)) != NULL)
	{
		add_station(prev, next - prev);
		size -= (next - prev + 1);
		prev  = (next + 1);
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
		errx(1, "Usage: %s <file>", argv[0]);

	open_file(argv[1]);
	do_read();

	qsort(stations, HT_SIZE,
		sizeof(struct station), &cmp_string);

	list_stations();
	close_file();
	return (0);
}
