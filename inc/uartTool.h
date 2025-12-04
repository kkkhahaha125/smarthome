#ifndef __UARTTOOL_H
#define __UARTTOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define SERIAL_DEV "/dev/ttyS5"
#define BAUD 115200

int serialOpen (const char *device, const int baud);

void serialPutString (const int fd, const unsigned char *s, int len);

int serialGetString (const int fd, unsigned char *buffer);


#endif
