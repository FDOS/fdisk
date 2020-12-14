#ifndef _GETOPT_H
#define _GETOPT_H

#ifdef __cplusplus
extern "C" {
#endif

extern char    *optarg;        /* Global argument pointer. */
extern int     optind;         /* Global argv index. */

int getopt(int argc, char *argv[], char *optstring);

#ifdef __cplusplus
}
#endif

#endif
