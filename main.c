#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include "serial.h"
#include "config.h"
#include "device.h"

void usage(char *a0)
{
	fprintf(stderr, "usage: %s [options]\n", a0);
	fprintf(stderr, "  chip <name>       (default: at89c2051)\n");
	fprintf(stderr, "  port <port>       (default: /dev/seprog)\n");
	fprintf(stderr, "  offset <value>\n");
	fprintf(stderr, "  size <value>\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  erase\n");
	fprintf(stderr, "  check\n");
	fprintf(stderr, "  read <filename>\n");
	fprintf(stderr, "  write <filename>\n");
	fprintf(stderr, "  fuse <value>\n");
}

int main(int argc, char **argv)
{
	int i, offset = 0, size = 0;
	const char *port = "/dev/seprog";

	if (argc < 2) {
		usage(argv[0]);
		exit(1);
	}

	chip = &chips[1];

	for (i = 1; i < argc; i++) {
		if (!strncasecmp(argv[i], "chip", strlen(argv[i])) && (i + 1) < argc) {
			int j;

			i++;

			chip = NULL;

			for (j = 0; chips[j].name; j++) {
				if (!strcmp(chips[j].name, argv[i])) {
					chip = &chips[j];
					break;
				}

				if (strstr(chips[j].name, argv[i])) {
					if (chip) {
						fprintf(stderr, "Chip name \"%s\" in ambigous\n", argv[i]);
						exit(1);
					} else
						chip = &chips[j];
				}
			}

			offset = 0;
			size = chip->size;

			continue;
		}

		if (!strncasecmp(argv[i], "port", strlen(argv[i])) && (i + 1) < argc) {
			i++;

			serial_close();
			
			port = argv[i];
			
			continue;
		}

		if (!strncasecmp(argv[i], "offset", strlen(argv[i])) && (i + 1) < argc) {
			i++;

			offset = strtol(argv[i], NULL, 0);

			printf("offset = %.4x\n", offset);
			
			continue;
		}

		if (!strncasecmp(argv[i], "size", strlen(argv[i])) && (i + 1) < argc) {
			i++;

			size = strtol(argv[i], NULL, 0);

			printf("size = %.4x\n", size);
			
			continue;
		}

		if (!strncasecmp(argv[i], "check", strlen(argv[i]))) {
			int result;

			serial_open(port);
			
			printf("Checking %s...\n", chip->name);
			
			result = chip_blank_check(0, -1);

			if (result == -1)
				printf("Blank\n");
			else
				printf("Not blank at offset %.4x\n", result);
			
			continue;
		}	

		if (!strncasecmp(argv[i], "erase", strlen(argv[i]))) {
			int result;
			
			serial_open(port);
			
			printf("Erasing %s...\n", chip->name);
			
			result = chip_erase();

			if (result == -1)
				printf("Erased\n");
			else
				printf("Not blank at offset %.4x\n", result);
			
			continue;
		}

		if (!strncasecmp(argv[i], "read", strlen(argv[i])) && (i + 1) < argc) {
			char *buf = malloc(size);
			int fd;

			i++;

			serial_open(port);
			
			printf("Reading %s (offset=0x%.4x, size=0x%.4x)...\n", chip->name, offset, size);

			chip_read(0, size, buf);

			printf("Read\n");
		
			if (!argv[i]) {
				fprintf(stderr, "Need filename");
				goto cleanup;
			}

			unlink(argv[i]);

			if ((fd = open(argv[i], O_WRONLY | O_CREAT, 0644)) == -1) {
				perror(argv[i]);
				goto cleanup;
			}
				
			if (write(fd, buf, size) < size) {
				perror(argv[i]);
				goto cleanup;
			}

			close(fd);
		
			continue;
		}

		if (!strncasecmp(argv[i], "fuse", strlen(argv[i])) && (i + 1) < argc) {
			int value;

			i++;

			serial_open(port);

			value = strtol(argv[i], NULL, 0);
			
			printf("Fusing %s (value=0x%.2x)...\n", chip->name, value);

			chip_fuse(value);

			printf("Done\n");
	
			continue;
		}

		if (!strncasecmp(argv[i], "write", strlen(argv[i])) && (i + 1) < argc) {
			char *buf = malloc(chip->size);
			int fd, size, result;

			i++;

			serial_open(port);
			
			if (!argv[i]) {
				fprintf(stderr, "Need filename");
				goto cleanup;
			}

			fd = open(argv[i], O_RDONLY);
			size = read(fd, buf, chip->size);
			close(fd);

			printf("Writing %s (offset=0x%.4x, size=0x%.4x)...\n", chip->name, offset, size);
	
			result = chip_write(offset, size, buf);
	
			if (result != size)
				printf("Error at offset %.4x\n", result);
			else
				printf("Written");
		
			continue;
		}
	}

cleanup:
	serial_close();
	
	return 0;
}
