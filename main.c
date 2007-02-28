#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#define _GNU_SOURCE
#include <getopt.h>
#include "serial.h"
#include "config.h"
#include "device.h"
#include "main.h"

void usage(char *a0)
{
	char buf[4096];
	int i, j;

	strcpy(buf, "  ");
	j = 2;

	for (i = 0; chips[i].name; i++) {
		if (strlen(chips[i].name) + 4 + j > 80) {
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "\n  ");
			j = 2;
		}

		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s ", chips[i].name);
		j += strlen(chips[i].name) + 1;
	}

	fprintf(stderr,
"Usage: %s [OPTIONS]...\n"
"\n"
"  -c, --chip=NAME     (default: %s)\n"
"  -p, --port=PORT     (default: " DEFAULT_DEVICE ")\n"
"  -o, --offset=VALUE\n"
"  -s, --size=VALUE\n"
"\n"
"  -e, --erase\n"
"  -b, --black-check\n"
"  -r, --read=FILE\n"
"  -w, --write=FILE\n"
"  -f, --fuse=VALUE\n"
"\n"
"  -v, --verbose\n"
"  -V, --version\n"
"  -h, --help\n"
"\n"
"Available chips:\n"
"%s\n"
"\n"
, a0, chips[DEFAULT_CHIP].name, buf);
}


static struct option long_options[] = {
	{ "chip", 1, 0, 'c' },
	{ "port", 1, 0, 'p' },
	{ "offset", 1, 0, 'o' },
	{ "size", 1, 0, 's' },
	{ "erase", 0, 0, 'e' },
	{ "blank-check", 0, 0, 'b' },
	{ "read", 1, 0, 'r' },
	{ "write", 1, 0, 'w' },
	{ "fuse", 1, 0, 'f' },
	{ "verbose", 0, 0, 'v' },
	{ "version", 0, 0, 'V' },
	{ "help", 0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
	int c, optind2, offset = 0, size = 0;
	const char *port = DEFAULT_DEVICE;

	if (argc < 2) {
		usage(argv[0]);
		exit(1);
	}

	chip = &chips[DEFAULT_CHIP];

	while ((c = getopt_long(argc, argv, "c:p:o:s:ebr:w:f:vVh", long_options, &optind2)) != -1) {
		switch (c) {
			case 'c':
			{
				int i;

				chip = NULL;

				for (i = 0; chips[i].name; i++) {
					if (!strcmp(chips[i].name, optarg)) {
						chip = &chips[i];
						break;
					}

					if (strstr(chips[i].name, optarg)) {
						if (chip) {
							fprintf(stderr, "Chip name \"%s\" in ambigous\n", optarg);
							exit(1);
						} else
							chip = &chips[i];
					}
				}
				
				offset = 0;
				size = chip->size;

				break;
			}

			case 'p':
			{
				serial_close();
				port = optarg;
				break;
			}

			case 'o':
			{
				char *endptr;

				offset = strtol(optarg, &endptr, 0);

				if (*endptr) {
					fprintf(stderr, "Invalid offset \"%s\"\n", optarg);
					goto cleanup;
				}

				break;
			}

			case 's':
			{
				char *endptr;

				size = strtol(optarg, &endptr, 0);

				if (*endptr) {
					fprintf(stderr, "Invalid size \"%s\"\n", optarg);
					goto cleanup;
				}

				break;
			}
			
			case 'b':
			{
				int res;

				serial_open(port);
			
				printf("Checking %s...\n", chip->name);
			
				if ((res = chip_blank_check(0, -1)) == -1)
					printf("Blank\n");
				else
					printf("Not blank at offset %.4x\n", res);
				break;
			}

			case 'e':
			{
				int res;

				serial_open(port);
				
				printf("Erasing %s...\n", chip->name);
			
				if ((res = chip_erase()) == -1) 
					printf("Done\n");
				else
					printf("Not blank at offset %.4x\n", res);
				
				break;
			}

			case 'r':
			{
				unsigned char *buf = malloc(size);
				FILE *fp;

				serial_open(port);
			
				printf("Reading %s (offset=0x%.4x, size=0x%.4x)...\n", chip->name, offset, size);

				chip_read(0, size, buf);

				printf("Done\n");
		
				if (!optarg) {
					fprintf(stderr, "Need filename\n");
					goto cleanup;
				}
				
				if (unlink(optarg) == -1) {
					perror(optarg);
					goto cleanup;
				}
				
				if (!(fp = fopen(optarg, "wb"))) {
					perror(optarg);
					goto cleanup;
				}
				
				if (fwrite(buf, 1, size, fp) < size) {
					perror(optarg);
					fclose(fp);
					goto cleanup;
				}

				fclose(fp);

				break;
			}
			
			case 'f':
			{
				char *endptr;
				int value;

				serial_open(port);
				
				value = strtol(optarg, &endptr, 0);
			
				if (*endptr) {
					fprintf(stderr, "Invalid fuse \"%s\"\n", optarg);
					goto cleanup;
				}

				printf("Fusing %s (value=0x%.2x)...\n", chip->name, value);

				chip_fuse(value);
	
				printf("Done\n");
		
				break;
			}

			case 'w':
			{
				unsigned char *buf = malloc(chip->size);
				int size, result;
				FILE *fp;

				serial_open(port);
			
				if (!optarg) {
					fprintf(stderr, "Need filename\n");
					goto cleanup;
				}
				
				if (!(fp = fopen(optarg, "rb"))) {
					perror(optarg);
					goto cleanup;
				}
				
				size = fread(buf, 1, chip->size, fp);
				fclose(fp);

				printf("Writing %s (offset=0x%.4x, size=0x%.4x)...\n", chip->name, offset, size);
	
				result = chip_write(offset, size, buf);
	
				if (result != size)
					printf("Error at offset %.4x\n", result);
				else
					printf("Done\n");
				
				break;
			}

			case 'h':
			{
				usage(argv[0]);
				exit(0);
			}

			case 'v':
			{
				verbose++;
				break;
			}

			case 'V':
			{
				printf("seprog %s\n", VERSION);
				break;
			}

			default:
			{
				usage(argv[0]);
				exit(1);
			}
		}
	}

cleanup:
	serial_close();
	
	return 0;
}
