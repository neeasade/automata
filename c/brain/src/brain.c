#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
 * constants
 */
static char *COLORS[3] = {
    "016 032 041",
    "113 170 197",
    "201 100 126"
};

/*
 * helper functions
 */
static void
usage(char *name)
{
    fprintf(
        stderr,
        "usage : %s [-whi <number>]\n\n"
        "options :\n"
        "    -w <number>   set width  to <number> (default : 500)\n"
        "    -h <number>   set height to <number> (default : 500)\n"
        "    -i <number>   iterate <number> times (default : 500)\n",
        basename(name));

    exit(1);
}

static unsigned
convert_to_number(const char *str)
{
    errno = 0;

    char *ptr;
    long number;

    if ((number = strtol(str, &ptr, 10)) < 0 || errno != 0 || *ptr != 0)
        errx(1, "'%s' isn't a valid positive integer", str);

    return number;
}

static void *
allocate(size_t size)
{
    void *ptr;

    if ((ptr = calloc(1, size)) == NULL)
        errx(1, "failed to allocate memory");

    return ptr;
}

static inline unsigned
wrap(unsigned coord, long offset, unsigned bound)
{
    return ((long)coord + offset + bound) % bound;
}

/*
 * main function
 */
static void
brain(unsigned width, unsigned height, unsigned iter)
{
    /* init array */
    unsigned short ***uni;

    uni = allocate(height * sizeof(*uni));

    for (unsigned i = 0; i < height; ++i) {
        uni[i] = allocate(width * sizeof(*uni[i]));

        for (unsigned j = 0; j < width; ++j)
            uni[i][j] = allocate(2 * sizeof(*uni[i][j]));
    }

    bool flag = 0;

    for (unsigned i = 0; i < width * height / 4; ++i)
        uni[rand() % height][rand() % width][flag] = rand() % 2 + 1;

    /* run brian's brain */
    unsigned short tmp;
    unsigned short cnt;

    for (unsigned n = 0; n < iter; ++n) {
        printf("P3\n%u %u\n255\n", width, height);

        for (unsigned i = 0; i < height; ++i)
            for (unsigned j = 0; j < width; ++j) {
                tmp = uni[i][j][flag];

                if (n == (iter - 1)) {
                    puts(COLORS[tmp]);
                }

                if (tmp == 0) {
                    cnt = 0;

                    for (short k = -1; k <= 1; ++k)
                        for (short l = -1; l <= 1; ++l)
                            if (k != 0 || l != 0)
                                if (uni[wrap(i, k, height)][wrap(j, l, width)][flag] == 2)
                                    ++cnt;

                    tmp = cnt == 2 ? 2 : 0;
                }
                else
                    --tmp;

                uni[i][j][!flag] = tmp;
            }

        flag ^= 1;
    }

    /* cleanup */
    for (unsigned i = 0; i < height; ++i) {
        for (unsigned j = 0; j < width; ++j)
            free(uni[i][j]);

        free(uni[i]);
    }

    free(uni);
}

int
main(int argc, char **argv)
{
    /* argument parsing */
    unsigned width  = 500;
    unsigned height = 500;
    unsigned iter   = 500;
    unsigned int seed   = 1337;

    COLORS[0] = getenv("COLOR0");
    COLORS[1] = getenv("COLOR1");
    COLORS[2] = getenv("COLOR2");

    for (int arg; (arg = getopt(argc, argv, ":w:h:i:")) != -1;)
        switch (arg) {
            case 'w': width  = convert_to_number(optarg); break;
            case 'h': height = convert_to_number(optarg); break;
            case 'i': iter   = convert_to_number(optarg); break;
            case 'r': seed   = convert_to_number(optarg); break;
            default :
                usage(argv[0]);
        }

    if (optind < argc)
        usage(argv[0]);

    if (seed == 1337) {
        srand(time(NULL));
    } else {
        srand(seed);
    }

    brain(width, height, iter);

    return 0;
}
