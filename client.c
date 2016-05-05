#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/*
 * Open connection with server on given address using specific name and room.
 * Return value: socket fd on success or -1 on connection failure
 * (in this case check errno for details).
 */
int msgr_connect(const char *addr, const char *name, const char *room)
{
	int len;
	int sockfd;
	int status;
	struct sockaddr_in address;
	struct clnt_info inf;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		return -1;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(addr);
	address.sin_port = htons(MSGR_PORT);
	len = sizeof(address);

	status = connect(sockfd, (struct sockaddr *)&address, len);
	if (status == -1)
		return -1;

	strncpy(inf.name, name, STR_LNGTH);
	strncpy(inf.room, room, STR_LNGTH);

	status = write(sockfd, &inf, sizeof(inf));
	if (status == -1)
		return -1;

	return sockfd;
}

int main(int argc, char const *argv[])
{
	int sockfd;

	sockfd = msgr_connect("127.0.0.1", "user", "testroom");
	if (sockfd == -1) {
		perror("client");
		exit(EXIT_FAILURE);
	}

	printf("Connection successfull, closing now.\n");

	close(sockfd);
	exit(EXIT_SUCCESS);
}