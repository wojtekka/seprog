#ifndef __DEVICE_H
#define __DEVICE_H

unsigned char serial_read();
int serial_read_offset();
void serial_read_flush();
void serial_write_nak(int count, ...);
void serial_write_ack(int count, ...);
void serial_open(const char *device);
void serial_close();
void serial_error(const char *desc);

#endif
