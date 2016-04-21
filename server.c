#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

int main(void)
{
	int smpl_sckt;
	int status;

	struct in_addr in_ip;
	struct sockaddr_in in_sock;

/* information for binding */
	in_ip.s_addr = htonl(INADDR_ANY);
	in_sock.sin_family = AF_INET;
	in_sock.sin_port = htons(1488);
	in_sock.sin_addr = in_ip;

/* first step is to make simple socket */
	smpl_sckt = socket(AF_INET, SOCK_STREAM, 0);
	if (smpl_sckt == -1) {
		perror("Can't create simple socket");
		return -1;
	}

/* binding socket */
	status = bind(smpl_sckt, (struct sockaddr *) &in_sock, sizeof(in_sock));
	if (status == -1) {
		perror("Can't bind simple socket");
		return -1;
	}

}