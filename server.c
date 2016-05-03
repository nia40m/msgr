#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <common.h>


char ** clnt_nms = NULL;
uint16_t clnt_nmbr;

char ** room_nms = NULL;
uint16_t room_nmbr;


int init(int desc)
{
	int i;
	struct clnt_info new;
/* проверяем имя, не занято ли
проверяем комнату, существует или создавать
*/
	read(desc, &new, sizeof(struct clnt_info));

	for (i=0; i < clnt_nmbr; i++)
		/* доделать если слово занято !!!!!!!!!!!*/
		if (!strcmp(new.name, *(clnt_nms + i))) {
			printf("This nickname is bussy.\n");
			return -1;
		}

	/* creation of new client name*/
	clnt_nms = (char **) realloc(clnt_nms, clnt_nmbr + 1);
	/*CHECK POINTER TO NULL*/
	*(clnt_nms + clnt_nmbr) = (char *) calloc(STR_LNGTH, 1);
	/*CHECK POINTER TO NULL*/
	strcpy(*(clnt_nms + clnt_nmbr), new.name);
	clnt_nmbr++;

	for (i=0; i < room_nmbr; i++)
		if (!strcmp(new.room, *(room_nms + i))) goto room_exist;

	/* creation of new room name*/
	room_nms = (char **) realloc(room_nms, room_nmbr + 1);
	/*CHECK POINTER TO NULL*/
	*(room_nms + room_nmbr) = (char *) calloc(STR_LNGTH, 1);
	/*CHECK POINTER TO NULL*/
	strcpy(*(room_nms + room_nmbr), new.room);
	room_nmbr++;ды

room_exist:
	/* привязка к комнате */
}


void * client(void * param)
{
	/*нужен ли семафор? да*/
	int state;
	int clnt_desc = *(int *) param;

	/* initialization of client */
	state = init(clnt_desc);
	if (state = -1) return -1;
}


int main(void)
{
	int smpl_sckt;
	int incom_sckt;
	int status;

	struct in_addr ip;
	struct sockaddr_in srvr_sckt;
	struct sockaddr_in clnt_sckt;
	socklen_t sckt_lngth = (socklen_t) sizeof(struct sockaddr_in);

	/* information for binding */
	ip.s_addr = htonl(INADDR_ANY);
	srvr_sckt.sin_family = AF_INET;
	srvr_sckt.sin_port = htons(1488);
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

	/* listenning to incoming connections */
	incom_sckt = accept(smpl_sckt, (struct sockaddr *) &clnt_sckt,
		&sckt_lngth);
	if (incom_sckt == -1) {
		perror("Can't accept incoming socket");
		close(smpl_sckt);
		return -1;
	}
	/*semaforo*/

	/*struct clnt_info input;

	read(incom_sckt, &input, sizeof(struct clnt_info));

	printf("Client name: %s\n He wants to make a room: %s\n", input.name, input.room);

	getchar();*/

	/* close socket */
	close(smpl_sckt);
}
