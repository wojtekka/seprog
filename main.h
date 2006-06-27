#ifndef __MAIN_H
#define __MAIN_H

#ifndef WIN32
#define DEFAULT_DEVICE "/dev/seprog"
#else
#define DEFAULT_DEVICE "COM1"
#endif

#define DEFAULT_CHIP 1

#define VERSION "0.3"

int verbose;

#endif
