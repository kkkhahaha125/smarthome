#ifndef __SOCKET_H
#define __SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define IPADDR "192.168.43.59"
#define PORT   "8192"   //不能是8080，因为已经被摄像头进程使用



int socket_init(const char *ipaddr, const char *port);

#endif 


