#ifndef WIN32

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

#else /* WIN32 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

static HANDLE sfd = NULL;

#endif /* WIN32 */

#define debug(x, y...) do { } while(0)

/*
 * serial_read()
 *
 * czyta znak z portu szeregowego. w przypadku timeoutu wyjdzie z programu.
 */
unsigned char serial_read()
{
#ifndef WIN32

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
	
#else /* WIN32 */
	
	unsigned char res;
	unsigned long int numread;
	int ret;

	ret = ReadFile(sfd, &res, 1, &numread, NULL);
	
	if (!ret) {
		fprintf(stderr, "ReadFile failed! (code: %ld)\n", GetLastError());
		exit(1);
	}
	
	if (!numread) {
		fprintf(stderr, "No data on serial port! Cable problem!?\n");
		exit(1);
	}
	
	return res;
	
#endif /* WIN32 */
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
#ifndef WIN32
	int i, count;

	ioctl(fd, FIONREAD, &count);

	for (i = 0; i < count; i++)
		serial_read();
#else
	PurgeComm(sfd, PURGE_RXCLEAR);
#endif
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
#ifdef WIN32
	unsigned long int written;
#endif

	va_start(ap, count);

	for (i = 0; i < count; i++) {
		unsigned char ch;

		ch = (unsigned char) va_arg(ap, int);
		debug("write %.2x\n", ch);
#ifndef WIN32
		write(fd, &ch, 1);
#else
		WriteFile(sfd, &ch, 1, &written, NULL);
#endif
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
#ifdef WIN32
	unsigned long int written;
#endif

	va_start(ap, count);

	for (i = 0; i < count; i++) {
		unsigned char ch;

		ch = (unsigned char) va_arg(ap, int);
		debug("write %.2x\n", ch);
#ifndef WIN32
		write(fd, &ch, 1);
#else
		WriteFile(sfd, &ch, 1, &written, NULL);
#endif
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
#ifndef WIN32
	struct termios newtio;

	if (fd != -1)
		return;

	if ((fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1) {
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

#else /* WIN32 */

	DCB serial_dcb;
	COMMTIMEOUTS ctimeo;

	if (sfd != NULL)
		return;

	sfd = CreateFile(device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (sfd == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "serial_open: CreateFile(%s, ...) failed!, port not existant? device already opened?\n", device);
		exit(1);
	}

	memset(&serial_dcb, 0, sizeof(DCB));

	if (!GetCommState(sfd, &serial_dcb)) {
		fprintf(stderr, "serial_open: GetCommState failed!\n");
		CloseHandle(sfd);
		exit(1);
	}
	
	serial_dcb.BaudRate = CBR_57600;
	serial_dcb.Parity = NOPARITY;
	serial_dcb.StopBits = ONESTOPBIT;
	serial_dcb.ByteSize = 8;
	serial_dcb.fBinary = TRUE;
	serial_dcb.fParity = FALSE;
	serial_dcb.fOutxDsrFlow = FALSE; /* dsr influences outgoing traffic [y/N] */
	serial_dcb.fDtrControl = DTR_CONTROL_ENABLE;
	serial_dcb.fDsrSensitivity = FALSE; /* dsr influences incoming traffic [y/N] */
	serial_dcb.fOutX = FALSE; /* XON/XOFF on output - [y/N] */
	serial_dcb.fInX = FALSE; /* XON/XOFF on input - [y/N] */
	serial_dcb.fNull = FALSE; /* discard null bytes on input */
	serial_dcb.fRtsControl = RTS_CONTROL_ENABLE;
	serial_dcb.fAbortOnError = TRUE; /* see ClearCommError */

	if (!SetCommState(sfd, &serial_dcb)) {
		fprintf(stderr, "serial_open: SetCommState failed!\n");
		CloseHandle(sfd);
		exit(1);
	}

	memset(&ctimeo, 0, sizeof(COMMTIMEOUTS));
	ctimeo.ReadIntervalTimeout = 20;
	ctimeo.ReadTotalTimeoutMultiplier = 20;
	ctimeo.ReadTotalTimeoutConstant = 5000;
	SetCommTimeouts(sfd, &ctimeo);

	debug("serial_open: %s opened O.K.!\n", device);

#endif /* WIN32 */
}

/*
 * serial_close()
 *
 * zamyka port szeregowy.
 */ 
void serial_close()
{
#ifndef WIN32
	if (fd == -1)
		return;

	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
#else
	if (sfd == NULL)
		return;

	CloseHandle(sfd);
	sfd = NULL;
#endif
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


