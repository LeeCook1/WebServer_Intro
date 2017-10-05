#include <stdio.h>
#include <stdlib.h>		//exit
#include <string.h>		//memset

#include <sys/socket.h>		//socket, setsockopt
#include <netinet/in.h>		// struct sockaddr_in
#include <arpa/inet.h>		//inet_ntop

#include <sys/time.h>		// struct timeval
#include <sys/select.h>		//FD_... , select

#define PORT 7890
void hexdump(const unsigned char *buffer, const unsigned int length);
void connection_handle(int socket_fd, fd_set * fdset);

/* Server Accepts Multiple Connections and monitor them w/ select
 * 	Inefficient in that one would have to loop over all fds to see which one changed
 *	Also there exist a limit of fd's that can be used, roughly 1024, which mean
 *	only 1024 clients
 * */

int
main(int argc, char *argv[])
{
	char buff[1024];
	int sock_fd, newsock_fd, max_fd, select_v, i;
	int optval_yes = 1;
	socklen_t sin_size;
	struct sockaddr_in my_addr, client_addr;	//address info

	fd_set read_fds, socket_fds;
	struct timeval tv;

	/* Create Socket */

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("[!!] Fatal, Error creating socket");
		exit(-1);
	}

	/* Set Socket Options */

	if (setsockopt
	    (sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval_yes,
	     sizeof (int)) == -1) {
		perror("[!!] Fatal, Setting socket option SO_REUSEADDR!");
		exit(-1);
	}

	/* Set up host address info in sockaddr_in struct */

	memset(&my_addr, '\0', sizeof (struct sockaddr_in));

	my_addr.sin_family = AF_INET;	// IPv4
	my_addr.sin_port = htons(PORT);	// sin_port is short, switch to network-byte order

	my_addr.sin_addr.s_addr = 0;	// auto-fill my address

	/* bind() Assigns address to socket via file descriptor */
	if (bind
	    (sock_fd, (struct sockaddr *) &my_addr,
	     sizeof (struct sockaddr)) == -1) {
		perror("[!!] Fatal, binding socket!");
		exit(-1);
	}

	/* listen() Tell socket to start listening for connections 
	 * places all incoming connections into a backlog queue until accept() call accepts the connections
	 */
	if (listen(sock_fd, 3) == -1) {
		perror("[!!] Fatal, listening on socket");
		exit(-1);
	}
	printf("Waiting for a connection....\n");

	/* Set up for select */
	FD_ZERO(&socket_fds);
	FD_SET(sock_fd, &socket_fds);
	max_fd = sock_fd;

	/* Select loop */

	while (1) {

		//Linux implementation of select tampers with timeeval and fd_set
		//so i need to reset them everytime select runs

		tv.tv_sec = 5;
		tv.tv_usec = 0;
		read_fds = socket_fds;
		if ((select_v =
		     select(max_fd + 1, &read_fds, NULL, NULL, &tv)) == -1) {
			perror("[!!] Fatal, Select");
			exit(-1);
		} else if (select_v == 0) {
			printf("Timeout!\n");

			continue;
		}

		for (i = 0; i <= max_fd; i++) {
			printf("Check descriptor:%d\n", i);
			if (FD_ISSET(i, &read_fds)) {
				if (i == sock_fd)	//someone trying to connect
				{
					printf("Trying to conect\n");
					sin_size = sizeof (struct sockaddr_in);

					/* extract connection from queue, create new fd for connected socket */
					if ((newsock_fd =
					     accept(sock_fd,
						    (struct sockaddr *)
						    &client_addr,
						    &sin_size)) == -1) {
						perror
						    ("[!!] Fatal, Accepting connection");
						error(-1);
					}

					FD_SET(newsock_fd, &socket_fds);

					if (newsock_fd > max_fd)
						max_fd = newsock_fd;

					char str[INET_ADDRSTRLEN];
					inet_ntop(AF_INET,
						  &(client_addr.sin_addr), str,
						  INET_ADDRSTRLEN);
					//Print client info using inet_ntoa, ntoh network to ascii, network to host
					printf
					    ("Server: got connection from %s, at port %d\n",
					     str, ntohs(client_addr.sin_port));

					// Talk to socket
					send(newsock_fd, "My first socket!\n",
					     17, 0);

				} else	// data from a client
				{
					connection_handle(i, &socket_fds);
				}

			}
		}
	}

	return 0;
}

void
connection_handle(int socket_fd, fd_set * fdset)
{
	int recv_length = 1;
	char buff[1024];
	bzero(buff, sizeof (buff));
	if ((recv_length = recv(socket_fd, &buff, 1024, 0)) > 0) {
		printf("RECV: %d bytes\n", recv_length);
		hexdump(buff, recv_length);
		return;
	} else if (recv_length == -1) {
		perror("[!!] Fatal, Receiving From Client!");
		exit(-1);
	}
	//runs when recv_length=0 need to clear socket_fd and close it
	FD_CLR(socket_fd, fdset);
	close(socket_fd);
	return;
}

void
hexdump(const unsigned char *buffer, const unsigned int length)
{
	unsigned char byte;
	unsigned int i, j;

	for (i = 0; i < length; i++) {
		byte = buffer[i];
		printf("%02x ", buffer[i]);

		if ((i % 16 == 15) || (i == length - 1)) {
			for (j = 0; j < 15 - (i % 16); j++)
				printf(" _ ");
			printf("| ");

			/*display printable bytes */
			for (j = (i - (i % 16)); j <= i; j++) {
				byte = buffer[j];
				if ((byte > 31) && (byte < 127))
					printf("%c", byte);
				else
					printf(".");
			}

			printf("\n");
		}
	}
}

/*
//Returns true on success, or false if there was an error
int SetSocketBlockingEnabled(int fd, int blocking)
{
	  if (fd < 0) return 0;

#ifdef WIN32
	    unsigned long mode = blocking ? 0 : 1;
		  return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? 1 : 0;
#else
		    int flags = fcntl(fd, F_GETFL, 0);
			  if (flags < 0) return 0;
			    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
				  return (fcntl(fd, F_SETFL, flags) == 0) ? 1 : 0;
#endif
}

 */
