#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

static struct termios oldtio;
static int fd = -1;

#define debug(x, y...) do { } while(0)

/*
 * serial_read()
 *
 * czyta znak z portu szeregowego. w przypadku timeoutu wyjdzie z programu.
 */
unsigned char serial_read()
{
	fd_set rdfs;
	struct timeval tv;
	unsigned char res;

	FD_ZERO(&rdfs);
	FD_SET(fd, &rdfs);
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	if (!select(fd + 1, &rdfs, NULL, NULL, &tv)) {
		fprintf(stderr, "Communication timeout\n");
		exit(1);
	}

	if (read(fd, &res, 1) == 1) {
		debug("read %.2x\n", res);
		return res;
	} else {
		fprintf(stderr, "Communication error: %s\n", strerror(errno));
		exit(1);
	}
}

/*
 * serial_read_offset()
 *
 * czyta 24-bitowy offset z portu szeregowego. w razie timeoutu wyjdzie
 * z programu.
 */
int serial_read_offset()
{
	return ((serial_read() << 16) | (serial_read() << 8) | serial_read());
}

/*
 * serial_read_flush()
 *
 * czy¶ci bufor wej¶ciowy portu szeregowego, na wypadek gdyby czeka³y tam
 * jakie¶ nieodebrane przez program dane.
 */
void serial_read_flush()
{
	int i, count;

	ioctl(fd, FIONREAD, &count);

	for (i = 0; i < count; i++)
		serial_read();
}

/*
 * serial_write_nak()
 *
 * zapisuje @count bajtów do portu szeregowego nie czekaj±c na ich
 * potwierdzenie.
 */
void serial_write_nak(int count, ...)
{
	va_list ap;
	int i;

	va_start(ap, count);

	for (i = 0; i < count; i++) {
		unsigned char ch;

		ch = (unsigned char) va_arg(ap, int);
		debug("write %.2x\n", ch);
		write(fd, &ch, 1);
	}

	va_end(ap);
}

/*
 * serial_write_ack()
 *
 * zapisuje @count bajtów do portu szeregowego i czeka na ich potwierdzenie
 * z urz±dzenia. w razie braku potwierdzenia wychodzi z programu.
 */
void serial_write_ack(int count, ...)
{
	va_list ap;
	int i;

	va_start(ap, count);

	for (i = 0; i < count; i++) {
		unsigned char ch;

		ch = (unsigned char) va_arg(ap, int);
		debug("write %.2x\n", ch);
		write(fd, &ch, 1);
	}

	va_end(ap);

	va_start(ap, count);

	for (i = 0; i < count; i++) {
		unsigned char ch;

		ch = (unsigned char) va_arg(ap, int);
		
		if (ch != serial_read()) {
			fprintf(stderr, "Communication error: Invalid response\n");
			exit(1);
		}
	}
}

/*
 * serial_open()
 *
 * otwiera podany port szeregowy. w przypadku b³êdu wychodzi z programu.
 */
void serial_open(const char *device)
{
	struct termios newtio;

	if (fd != -1)
		return;

	if ((fd = open(device, O_RDWR | O_NOCTTY)) == -1) {
		perror(device);
		exit(1);
	}

	tcgetattr(fd, &oldtio);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = B57600 | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = PENDIN;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);
}

/*
 * serial_close()
 *
 * zamyka port szeregowy.
 */ 
void serial_close()
{
	if (fd == -1)
		return;

	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
}

/*
 * serial_error()
 *
 * wy¶wietla b³±d dotycz±cy komunikacji z urz±dzeniem i wychodzi z programu.
 */
void serial_error(const char *desc)
{
	fprintf(stderr, "Communication error: %s\n", desc);
	exit(1);
}

