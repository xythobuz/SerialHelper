/*
 * By: Thomas Buck <xythobuz.org>
 * Visit: www.xythobuz.org
 * SerialHelper:
 * A Crossplatform (Unixoids, Windows) utility to send data to and
 * reciceve data from a serial port.
 */

/*
 * Define DEBUG if you want to get debug output:
 */
#define DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "serial.h"

#ifndef PlatformWindows
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#define VERSION "0.5"

#ifdef PlatformWindows
#define PLATFORM "Windows"
#else
#define PLATFORM "Unix"
#endif

#ifndef DEFAULTSEARCH
#define DEFAULTSEARCH "tty"
#endif

#ifdef DEBUG
char debugText[] = "--> Debug:\t";
#endif

int keepRunning = 1;

#ifndef PlatformWindows
struct termios oldTerm;

int setRawMode(int fd) {
	struct termios newTerm;
	fcntl(fd, F_SETFL, FNDELAY);
	if (tcgetattr(fd, &oldTerm) == -1) {
#ifdef DEBUG
printf("%sError setting %x (A)\n", debugText, fd);
#endif
		return -1;
	}
	if (tcgetattr(fd, &newTerm) == -1) {
#ifdef DEBUG
printf("%sError setting %x (B)\n", debugText, fd);
#endif
		return -1;
	}
	newTerm = oldTerm;
	newTerm.c_lflag &= ~(ICANON | IEXTEN | ECHO);
	newTerm.c_iflag &= ~(ICRNL | ISTRIP | IXON);
	newTerm.c_cflag &= ~(CSIZE | PARENB);
	newTerm.c_cflag |= CS8;
	newTerm.c_oflag &= ~(OPOST);
	newTerm.c_cc[VMIN] = 0;
	newTerm.c_cc[VTIME] = 0;
	if (tcsetattr(fd, TCSANOW, &newTerm) == -1) {
#ifdef DEBUG
printf("%sError setting %x (C)\n", debugText, fd);
#endif
		return -1;
	}
	return 0;
}

int resetRawMode(int fd) {
	if (tcsetattr(fd, TCSAFLUSH, &oldTerm) == -1) {
#ifdef DEBUG
printf("%sError resetting %x\n", debugText, fd);
#endif
		return -1;
	}
	return 0;
}

#endif

void intHandler(int dummy) {
	keepRunning = 0;
}

int transmitFile(char *file) {
	char c;
	int character;

	FILE *fd = fopen(file, "r");
	if (fd == NULL) {
		printf("Error opening file!\n");
		serialClose();
		return -1;
	}

	while ((character = getc(fd)) != EOF) {
		c = (char)character;
		while (serialWrite(&c, 1) < 1);
	}
	fclose(fd);
	serialClose();
	return 0;
}

int main(int argc, char* argv[]) {
	
	int i = 0, c = 0, tmp;
	char ch;
	char *searchTerm = NULL;
	char **ports;
#ifndef PlatformWindows
	char *tempFileName = (char *)malloc((L_tmpnam + 1) * sizeof(char));
	char *commandLine = (char *)malloc((12 + L_tmpnam) * sizeof(char));
	char *osName = (char *)malloc(256 * sizeof(char));
	FILE *fp;

	strcpy(commandLine, "uname -s > ");
	tmpnam(tempFileName);
	strcat(commandLine, tempFileName);
#endif

	if (argc >= 2) {

		if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
			printf("SerialHelper for %s version %s\nThis utility allows you to send or recieve data to and from a serial port.\nYou can also get a list of available serial ports.\nIt is meant to be used as a tool to allow programs written in other languages access to serial ports.\n", PLATFORM, VERSION);

			printf("\nGetting a list of serial ports:\n\t%s -s [SearchTerm]\nDefault Search Term: \"%s\" or \"tty.\" on a Mac\n", argv[0], DEFAULTSEARCH);

			printf("\nSending data to a serial port:\n\t%s -t \"serial port\" \"Data to send...\"\n\t%s -tf \"serial port\" /file/to/send\n", argv[0], argv[0]);

			printf("\nRecieving data from a serial port:\n\t%s -r \"serial port\" length\n", argv[0]);

			printf("\nTerminal Mode (send stdin and recieve to stdout):\n\t%s -rw port\n", argv[0]);
			return 0;
		} else

		if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--version") == 0)) {
			printf("SerialHelper for %s version %s\n", PLATFORM, VERSION);
		} else

		if (strcmp(argv[1], "-r") == 0) {
			// Recieve argv[3] bytes via argv[2]
			if (argc != 4) {
				printf("Not enough / too much arguments!\n");
				return 1;
			}
			if (serialOpen(argv[2]) != 0) {
				printf("Error opening port!\n");
				return -1;
			}
			tmp = atoi(argv[3]);
			while (tmp > 0) {
				c = serialRead(&ch, 1);
				if (c > 0) {
					tmp -= c;
					putchar(ch);
				}
			}
			return 0;
		} else

		if (strcmp(argv[1], "-t") == 0) {
			// Transmit argv[3] via argv[2]
			if (argc != 4) {
				printf("Not enough / too much arguments!\n");
				return 1;
			}
			if (serialOpen(argv[2]) != 0) {
				printf("Error opening port!\n");
				return -1;
			}
			i = 0;
			tmp = strlen(argv[3]);
			while (i < tmp) {
				c = serialWrite(argv[3] + i, tmp - i);
				if (c != -1) {
					i += c;
				}
			}
			serialClose();
			return 0;
		} else

		if (strcmp(argv[1], "-tf") == 0) {
			// Transmit file argv[3] via argv[2]
			if (argc != 4) {
				printf("Not enough / too much arguments!\n");
				return 1;
			}
			if (serialOpen(argv[2]) != 0) {
				printf("Error opening port!\n");
				return -1;
			}
			return transmitFile(argv[3]);
		}

		if (strcmp(argv[1], "-s") == 0) {
			if (argc > 2)
				searchTerm = argv[2];
			if (searchTerm == NULL) {
#ifndef PlatformWindows
				// Detect Mac OS X and use "tty." as search term
				// Else use DEFAULTSEARCH
				if (system(commandLine) == 0) {
#ifdef DEBUG
printf("%sRan \"%s\" successfully\n", debugText, commandLine);
#endif
					fp = fopen(tempFileName, "r");
					if (fp != NULL) {
#ifdef DEBUG
printf("%sOpened \"%s\" successfully\n", debugText, tempFileName);
#endif
						fgets(osName, 256, fp);
						*strchr(osName, '\n') = '\0'; // Strip newline
#ifdef DEBUG
printf("%sGot OS Name \"%s\"\n", debugText, osName);
#endif
						if (strcmp(osName, "Darwin") == 0) {
							ports = getSerialPorts("tty.");
						} else {
							ports = getSerialPorts(DEFAULTSEARCH);
						}
						fclose(fp);
					} else {
						ports = getSerialPorts(DEFAULTSEARCH);
					}
				} else {
					ports = getSerialPorts(DEFAULTSEARCH);
				}
#else
				ports = getSerialPorts(NULL);
#endif
			} else {
				ports = getSerialPorts(searchTerm);
			}
			while (1) {
				if (ports[c] != NULL) {
					c++;
				} else {
					break;
				}
			}
			for (i = 0; i < c; i++) {
				printf("%s\n", ports[i]);
			}
		} else

		if (strcmp(argv[1], "-rw") == 0) {
			if (argc >= 3) {
				tmp = serialOpen(argv[2]);
				if (tmp == -1) {
					printf("Could not open port!\n");
					return -1;
				}
			} else {
				printf("No port name given!\n");
				return 1;
			}
			signal(SIGINT, intHandler);
#ifndef PlatformWindows
			signal(SIGQUIT, intHandler);
			setRawMode(STDIN_FILENO);
#endif
			printf("Connection established!\nClose with CTRL + C\n");
			while(keepRunning) {
				// Constantly read stdin and send it
				// Constantly write recieved data to stdout
				if (fread(&tmp, 1, 1, stdin) == 1) {
					serialWrite((char *)&tmp, 1);
				}
				if (serialRead((char *)&tmp, 1) == 1) {
					fwrite(&tmp, 1, 1, stdout);
				}
			}
			printf("\r\nClosing..\r\n");
#ifndef PlatformWindows
			resetRawMode(STDIN_FILENO);
#endif
			serialClose();
			return 0;
		} else {
			goto Usage;
		}

	} else {
Usage:
		printf("Help:\n%s -h\n", argv[0]);
		return 0;
	}

	return 0;
}