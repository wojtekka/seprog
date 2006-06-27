#ifndef __CHIPS_H
#define __CHIPS_H

#define VCC5 0x60
#define VCC12 0x61
#define VPP5 VCC5
#define VPP12 VCC12

struct chip {
	char *name;
	unsigned char type;
	unsigned long size;
	unsigned char bits;
	unsigned char vcc;
	unsigned char vpp;
	unsigned long delay;
};

extern struct chip chips[];

struct chip *chip;

#endif
	
