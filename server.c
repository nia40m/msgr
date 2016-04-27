#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

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
	incom_sckt = accept(smpl_sckt,
		(struct sockaddr *) &clnt_sckt,
		&sckt_lngth);
	if (incom_sckt == -1) {
		perror("Can't accept incoming socket");
		close(smpl_sckt);
		return -1;
	}

/* close socket */
	close(smpl_sckt);
}