#define _POSIX_C_SOURCE 2

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "common.h"

enum connect_error {
	CONN_FAIL = -1,		/* Connection failure (details in errno) */
	CONN_NAMEUSED = -2,	/* Name already in use */
	CONN_SRVERROR = -3,	/* Server error */
	CONN_ERROR = -4		/* Common connection error */
};

extern char *optarg;

int sockfd;

/*
 * Open connection with server on given address using specific name and room.
 * Return value:
 *	socket fd on success;
 *	CONN_FAIL on connection failure (in this case check errno for details);
 *	CONN_NAMEUSED if chosen name already in use;
 *	CONN_SRVERROR on server-side error;
 *	CONN_ERROR on common connection errors.
 */
int msgr_connect(const char *addr, const char *name, const char *room)
{
	int len;
	int sockfd;
	int status;
	char answer;
	struct sockaddr_in address;
	struct clnt_info inf;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		return CONN_FAIL;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(addr);
	address.sin_port = htons(MSGR_PORT);
	len = sizeof(address);

	status = connect(sockfd, (struct sockaddr *)&address, len);
	if (status == -1)
		return CONN_FAIL;

	strncpy(inf.name, name, STR_LNGTH);
	strncpy(inf.room, room, STR_LNGTH);

	status = write(sockfd, &inf, sizeof(inf));
	if (status == -1)
		return CONN_FAIL;

	status = read(sockfd, &answer, 1);
	if (status == -1)
		return CONN_FAIL;

	if (answer == ST_OK)
		return sockfd;

	/* Close socket on server deny */
	close(sockfd);

	switch (answer) {
	case ST_LBUSSY:
		return CONN_NAMEUSED;
	case ST_ERROR:
		return CONN_SRVERROR;
	}

	return CONN_ERROR;
}

void *reciever(void *arg)
{
	char str_msg[BUFSIZ];
	time_t cur_time;
	struct tm *tm_ptr;

	while (1) {
		int status = read(sockfd, &str_msg, BUFSIZ);

		if (status == -1) {
			perror("client reciever");
			break;
		} else if (status == 0) {
			printf("Connection closed by server\n");
			break;
		}

		time(&cur_time);
		tm_ptr = localtime(&cur_time);

		printf("[%02d:%02d:%02d] %s\n",
			tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec,
			str_msg);
	}

	return NULL;
}

int send_msg(const char *msg)
{
	/* Add 1 because strlen calculates length of string excluding '\0' */
	int status = write(sockfd, msg, strlen(msg) + 1);

	if (status == -1)
		return -1;

	return 0;
}

void *sender(void *arg)
{
	char buf[BUFSIZ];

	while (1) {
		if (fgets(buf, BUFSIZ, stdin) == NULL)
			break;
		if (buf[0] == '/') {
			if (strcmp(buf + 1, "exit\n") == 0)
				return NULL;
		}
		send_msg(buf);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int opt;
	pthread_t thread_reciever;
	pthread_t thread_sender;
	char addr[STR_LNGTH] = "";
	char name[STR_LNGTH] = "";
	char room[STR_LNGTH] = "";

	while ((opt = getopt(argc, argv, "a:n:r:")) != -1) {
		switch (opt) {
		case 'a':
			strncpy(addr, optarg, STR_LNGTH - 1);
			break;
		case 'n':
			strncpy(name, optarg, STR_LNGTH - 1);
			break;
		case 'r':
			strncpy(room, optarg, STR_LNGTH - 1);
			break;
		default:
			fprintf(stderr, "Usage: %s "
				"-a address -n name -r room\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/* Check for undefined arguments */
	if (*addr == '\0' || *name == '\0' || *room == '\0') {
		fprintf(stderr, "Usage: %s "
			"-a address -n name -r room\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	sockfd = msgr_connect(addr, name, room);
	switch (sockfd) {
	case CONN_FAIL:
		perror("client");
		exit(EXIT_FAILURE);
	case CONN_NAMEUSED:
		fprintf(stderr, "Requested username currently in use. "
			"Please choose another name and try again.\n");
		exit(EXIT_FAILURE);
	case CONN_SRVERROR:
		fprintf(stderr, "Server error.\n");
		exit(EXIT_FAILURE);
	case CONN_ERROR:
		fprintf(stderr, "Connection error.\n");
		exit(EXIT_FAILURE);
	}

	printf("Connection successfull\n");

	pthread_create(&thread_reciever, NULL, &reciever, NULL);
	pthread_create(&thread_sender, NULL, &sender, NULL);

	pthread_join(thread_sender, NULL);
	pthread_cancel(thread_reciever);

	close(sockfd);
	printf("Connection closed\n");
	exit(EXIT_SUCCESS);
}