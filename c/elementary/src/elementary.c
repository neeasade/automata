#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * helper functions
 */
static void
usage(char *name)
{
    fprintf(stderr, "usage : %s [rule] [strip]\n", basename(name));

    exit(1);
}

static unsigned short
get_rule(const char *str)
{
    errno = 0;

    char *ptr;
    long number;

    number = strtol(str, &ptr, 10);

    if (number > 255 || number < 0 || errno != 0 || *ptr != 0)
        errx(1, "'%s' isn't a valid rule (int between 0 and 255)", str);

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

/*
 * main function
 */
static void
elementary(unsigned short rule, const char *strip)
{
    /* start parsing strip */
    size_t length;


    /* length = strnlen(strip, LINE_MAX); */

    /* getconf ARG_MAX */
    /* 2505728 */

    /* 2x 4k */
    length = strnlen(strip, 7680);

    /* init array */
    bool **uni;
    bool flag = 0;

    uni = allocate(length * sizeof(*uni));

    for (size_t i = 0; i < length; ++i) {
        uni[i] = allocate(2 * sizeof(*uni[i]));

        switch (strip[i]) {
            case '0': /* fallthrough */
            case '1':
                uni[i][flag] = strip[i] - '0';

                break;
            default:
                errx(1, "'%c' invalid byte found in strip (int between 0 and 1)", strip[i]);
        }
    }

    /* run elementary automaton */
    unsigned short cnt;

    size_t skip = 300;
    printf("P6\n%lu %lu\n255\n", length, length - skip);

    int r, g, b;
    int r1, g1, b1;

    unsigned char rgb[3];
    unsigned char rgb1[3];

    /* COLORS[0] = getenv("COLOR0"); */
    /* COLORS[1] = getenv("COLOR1"); */

    sscanf(getenv("COLOR0"), "%d %d %d", &r, &g, &b);
    rgb[0] = (unsigned char)r;
    rgb[1] = (unsigned char)g;
    rgb[2] = (unsigned char)b;

    sscanf(getenv("COLOR1"), "%d %d %d", &r1, &g1, &b1);
    rgb1[0] = (unsigned char)r1;
    rgb1[1] = (unsigned char)g1;
    rgb1[2] = (unsigned char)b1;

    for (size_t i = 0; i < length; ++i) {
        for (size_t j = 0; j < length; ++j) {
            if (i >= skip) {
                if (uni[j][flag] == 0) {
                    fwrite(rgb, sizeof(unsigned char), 3, stdout);
                    /* putchar('0'); */
                } else {
                    fwrite(rgb1, sizeof(unsigned char), 3, stdout);
                    /* putchar('1'); */
                }
            }

            /* putchar(uni[j][flag] + '0'); */

            cnt = 0;

            for (short k = -1; k <= 1; ++k)
                cnt = cnt << 1 | uni[((long)j + k + length) % length][flag];

            uni[j][!flag] = 1 & rule >> cnt;
        }

        flag ^= 1;
    }

    /* cleanup */
    for (size_t i = 0; i < length; ++i)
        free(uni[i]);

    free(uni);
}

int
main(int argc, char **argv)
{
    if (argc != 3)
        usage(argv[0]);

    elementary(get_rule(argv[1]), argv[2]);

    fflush(stdout);

    return 0;
}
