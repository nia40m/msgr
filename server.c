#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>

#include "common.h"


struct names {
	char name[STR_LNGTH];
	struct names *next;
};

struct names *clients;


struct connection {
	char name[STR_LNGTH];
	char room[STR_LNGTH];
	int desc;
	struct connection *next;
};

struct connection *chat;


/* semaphore */
sem_t sem_accept;
sem_t sem_chat;


int lnames_add(char *whom, struct names **whereto)
{
	struct names *new;

	new = (struct names *) calloc(1, sizeof(struct names));
	if (new == NULL) {
		perror("Can't allocate memory to a new member of a list");
		return -1;
	}

	strcpy(new->name, whom);
	new->next = *whereto;
	*whereto = new;

	return 0;
}


int lnames_remove(char *whom, struct names **from)
{
	struct names *prev = NULL;
	struct names *curr = *from;
	struct names *next = curr->next;

	while (curr != NULL) {
		if (!strcmp(curr->name, whom)) {
			if (prev == NULL)
				*from = next;
			else
				prev->next = next;
			free(curr);

			return 0;
		}

		prev = curr;
		curr = next;
		next = curr->next;
	}

	return -1;
}


int lnames_find(char *whom, struct names **where)
{
	struct names *curr = *where;

	if (curr == NULL)
		return -1;

	while (curr != NULL) {
		if (!strcmp(curr->name, whom))
			return 0;

		curr = curr->next;
	}

	return -1;
}


struct connection *lconn_add(struct connection whom,
	struct connection **whereto)
{
	struct connection *new;

	new = (struct connection *) calloc(1, sizeof(struct connection));
	if (new == NULL) {
		perror("Can't allocate memory to a new member of a list");
		return NULL;
	}

	*new = whom;
	new->next = *whereto;
	*whereto = new;

	return new;
}


int lconn_remove(int whom, struct connection **from)
{
	struct connection *prev = NULL;
	struct connection *curr = *from;
	struct connection *next = curr->next;

	while (curr != NULL) {
		if (curr->desc == whom) {
			if (prev == NULL)
				*from = next;
			else
				prev->next = next;

			free(curr);

			return 0;
		}

		prev = curr;
		curr = next;
		next = curr->next;
	}

	return -1;
}


struct connection *init_client(int desc)
{
	int status;
	char response;
	struct clnt_info new;
	struct connection new_conn;
	struct connection *curr;

	/* obtain client login and room */
	status = read(desc, &new, sizeof(struct clnt_info));
	if (status <= 0)
		return NULL;

	/* checking login */
	status = lnames_find(new.name, &clients);

	/* login is already taken */
	if (status == 0) {
		response = ST_LBUSSY;
		write(desc, &response, 1);
		return NULL;
	}

	/* adding a new client */
	status = lnames_add(new.name, &clients);
	if (status == -1) {
		response = ST_ERROR;
		write(desc, &response, 1);
		return NULL;
	}

	/* copying name, room and descriptor */
	strcpy(new_conn.name, new.name);
	strcpy(new_conn.room, new.room);
	new_conn.desc = desc;
	new_conn.next = NULL;

	/* adding new object to chat list */
	curr = lconn_add(new_conn, &chat);
	if (curr == NULL) {
		response = ST_ERROR;
		write(desc, &response, 1);
		lnames_remove(new.name, &clients);
		return NULL;
	}

	response = ST_OK;
	status = write(desc, &response, 1);
	if (status <= 0) {
		lconn_remove(curr->desc, &chat);
		lnames_remove(new.name, &clients);
		return NULL;
	}

	return curr;
}


void room_send(char *msg, uint16_t len, char *room, struct connection **exept)
{
	struct connection *temp = chat;

	/* prevent of editing list of chat objects */
	sem_wait(&sem_chat);

	while (temp != NULL) {
		if (temp != *exept && !strcmp(room, temp->room))
			write(temp->desc, msg, len);

		temp = temp->next;
	}

	sem_post(&sem_chat);
}


void *client(void *param)
{
	uint16_t msg_len;
	uint16_t login_len;
	char buff[MSG_BUFF];
	struct connection *curr;
	int clnt_desc = *(int *) param;

	/* making main function to continue */
	sem_post(&sem_accept);

	/* initialization of client */
	sem_wait(&sem_chat);
	curr = init_client(clnt_desc);
	sem_post(&sem_chat);

	/* bad initialization */
	if (curr == NULL) {
		close(clnt_desc);
		pthread_exit(NULL);
	}

	/*
	 * writing client login to the string
	 * "+ 1" because sprintf doesn't count the \0 symbol
	 */
	login_len = sprintf(buff, "%s", curr->name) + 1;

	/*
	 * writing msg to the string
	 * "+ 1" because sprintf doesn't count the \0 symbol
	 */
	msg_len = sprintf(buff + login_len, "has joined the room") + 1;

	room_send(buff, login_len + msg_len, curr->room, &curr);

	while (1) {

		msg_len = read(clnt_desc, buff + login_len, BUFF_SIZE);

		/* check if client was disconnected or error reading */
		if (msg_len <= 0) {
			msg_len = sprintf(buff + login_len,
				"has left the room") + 1;

			room_send(buff, login_len + msg_len, curr->room, &curr);

			sem_wait(&sem_chat);
			lnames_remove(curr->name, &clients);
			lconn_remove(clnt_desc, &chat);
			sem_post(&sem_chat);

			close(clnt_desc);
			pthread_exit(NULL);
		}

		room_send(buff, login_len + msg_len, curr->room, &curr);
	}
}


int main(void)
{
	int smpl_sckt;
	int incom_sckt;
	int status;
	pthread_t thread_id;

	struct in_addr ip;
	struct sockaddr_in srvr_sckt;
	struct sockaddr_in clnt_sckt;
	socklen_t sckt_lngth = (socklen_t) sizeof(struct sockaddr_in);

	/* information for binding */
	ip.s_addr = htonl(INADDR_ANY);
	srvr_sckt.sin_family = AF_INET;
	srvr_sckt.sin_port = htons(MSGR_PORT);
	srvr_sckt.sin_addr = ip;

	/* first step is to make simple socket */
	smpl_sckt = socket(AF_INET, SOCK_STREAM, 0);
	if (smpl_sckt == -1) {
		perror("Can't create simple socket");
		return -1;
	}

	/* binding socket */
	status = bind(smpl_sckt, (struct sockaddr *) &srvr_sckt, sckt_lngth);
	if (status == -1) {
		perror("Can't bind simple socket");
		close(smpl_sckt);
		return -1;
	}

	/* making a queue for incoming connections */
	status = listen(smpl_sckt, 9);
	if (status == -1) {
		perror("Can't make queue");
		close(smpl_sckt);
		return -1;
	}

	/* initialization of semaphores */
	sem_init(&sem_accept, 0, 0);
	sem_init(&sem_chat, 0, 1);

	while (1) {
		/* listenning to incoming connections */
		incom_sckt = accept(smpl_sckt, (struct sockaddr *) &clnt_sckt,
			&sckt_lngth);
		if (incom_sckt == -1) {
			perror("Can't accept incoming socket");
			close(smpl_sckt);
			return -1;
		}

		/* starting a thread for a new client */
		pthread_create(&thread_id, NULL, &client, &incom_sckt);
		if (status != 0) {
			perror("Can't create thread for new client");
			close(smpl_sckt);
			return -1;
		}

		/* waiting for thread to save client info */
		sem_wait(&sem_accept);
	}
}
