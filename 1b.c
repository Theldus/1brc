/* Public domain. */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* Current version: v1. */

/***********************************************************************
 * Change log:
 * v1:
 *  Initial implementation:
 *   Pretty dumb, pretty straightforward, do _not_ use any kind of
 *   hashing, awfully slow (slower than the reference Java), but it
 *   works.
 *
 *
 */

static int    txt_fd;
static char  *txt_buff;
static off_t  txt_size;

static struct station
{
	char *name;
	float min;
	float avg;
	float max;
	int count;
} stations[10000] = {0};

static size_t total_stations = 0;

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
 * @brief Given the station line and its size, add it into the
 * memory.
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

	/* Iterate over the station array and check if there is
	 * an existing match, if so, change the values, otherwise,
	 * add a new entry.
	 */
	for (int i = 0; i < 10000; i++)
	{
		/* Empty entry, not found, add it. */
		if (!stations[i].name)
		{
			total_stations++;
			stations[i].avg   = value;
			stations[i].min   = value;
			stations[i].max   = value;
			stations[i].count = 1;

			stations[i].name = malloc(st_size + 1);
			memcpy(stations[i].name, station_line, st_size);
			stations[i].name[st_size] = '\0';
			break;
		}

		else if (!strncmp(station_line, stations[i].name, st_size)) {
			if (value < stations[i].min)
				stations[i].min = value;
			if (value > stations[i].max)
				stations[i].max = value;

			stations[i].avg += value;
			stations[i].count++;
			break;
		}
	}
}

/* String comparator. */
static int
cmpstringp(const void *p1, const void *p2)
{
	const struct station *s1, *s2;
	s1 = p1;
	s2 = p2;
	return (strcmp(s1->name, s2->name));
}

/*
 * List all the stations in the same output as the
 * challenge requires.
 */
static void list_stations(void)
{
	printf("{");
	for (size_t i = 0; i < total_stations; i++)
	{
		printf("%s=%.1f/%.1f/%.1f",
			stations[i].name,
			stations[i].min,
			stations[i].avg / (float)stations[i].count,
			stations[i].max,
			stations[i].count);

		if (i < total_stations - 1)
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

	qsort(stations, total_stations,
		sizeof(struct station), &cmpstringp);

	list_stations();
	close_file();
	return (0);
}
