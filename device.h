#ifndef __SEPROG_DEVICE_H
#define __SEPROG_DEVICE_H

int chip_erase();
int chip_blank_check(int offset, int size);
void chip_read(int offset, int size, unsigned char *buffer);
int chip_write(int offset, int size, unsigned char *buffer);
void chip_fuse(unsigned char value);

#endif

