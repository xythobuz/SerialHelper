/*
 * POSIX compatible serial port library
 * Uses 8 databits, no parity, 1 stop bit, no handshaking
 * By: Thomas Buck <xythobuz@me.com>
 * Visit: www.xythobuz.org
 */

// Use POSIX Baud constants (B2400, B9600...)
#define BAUD B19200
// Searchterm for ports in unix
#define SEARCH "tty"

// Open the serial port. Return 0 on success, -1 on error
int serialOpen(char *port);

// Write to port. Returns number of characters sent, -1 on error
ssize_t serialWrite(char *data, size_t length);

// Read from port. Return number of characters read, 0 if none available, -1 on error
ssize_t serialRead(char *data, size_t length);

// Close the serial Port
void serialClose(void);

// String array with serial port names
char** getSerialPorts(char *search);
