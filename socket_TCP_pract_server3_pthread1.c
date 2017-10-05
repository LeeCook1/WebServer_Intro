#include <stdio.h>
#include <stdlib.h> //exit
#include <string.h> //memset
#include <sys/socket.h> //socket, setsockopt
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> //inet_ntop

#include <pthread.h> //pthread_...

#define PORT 7890
void hexdump(const unsigned char *buffer, const unsigned int length );
void *connect_handle(void *arg);

/* Server Accept MultiConnections w/Fork
 * Introduces a race condition:
 *	If multiple connections come through simultaneously, the newsocket_fd may not be the apropriate one
 *	by the time the connection handlergets ran. This may cause some clients to not be handled or some clients get handled multiple times
 *
 */

int main(int argc, char *argv[])
{
	int sock_fd,newsock_fd;
	int recv_length=1,optval_yes=1;
	socklen_t sin_size;
	struct sockaddr_in my_addr, client_addr; //address info
	/* Create Socket */

	if ((sock_fd= socket(PF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("[!!] Fatal, Error creating socket");
		exit(-1);
	}

	/* Set Socket Options */

	if (setsockopt(sock_fd, SOL_SOCKET,SO_REUSEADDR,&optval_yes,sizeof(int) )==-1)
	{
		perror("[!!] Fatal, Setting socket option SO_REUSEADDR!");
		exit(-1);
	}

	/* Set up host address info in sockaddr_in struct */

	memset(&my_addr, '\0', sizeof(struct sockaddr_in));
	
	my_addr.sin_family= AF_INET; // IPv4
	my_addr.sin_port = htons(PORT); // sin_port is short, switch to network-byte order

	my_addr.sin_addr.s_addr= 0; // auto-fill my address


	/* bind() Assigns address to socket via file descriptor */
	if ( bind(sock_fd,(struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1 )
	{
		perror("[!!] Fatal, binding socket!");
		exit(-1);
	}
	
	/* listen() Tell socket to start listening for connections 
	 * places all incoming connections into a backlog queue until accept() call accepts the connections
	 */
	if (listen(sock_fd, 3)== -1)
	{
		perror("[!!] Fatal, listening on socket");
		exit(-1);
	}
	printf("Waiting for a connection....\n");
	/* The accept loop*/
	sin_size= sizeof(struct sockaddr_in);
	while(1)
	{
		/* extract connection from queue, create new fd for connected socket*/
		if((newsock_fd= accept(sock_fd, (struct sockaddr*)&client_addr, &sin_size))== -1 )
		{
			perror("[!!] Fatal, Accepting connection");
			error(-1);
		}
		
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(client_addr.sin_addr), str, INET_ADDRSTRLEN);
		//Print client info using inet_ntoa, ntoh network to ascii, network to host
		printf("Server: got connection from %s, at port %d\n",str, ntohs(client_addr.sin_port) );

		pthread_t tid;
		if(pthread_create(&tid,NULL,connect_handle, (void *)&newsock_fd ) == -1)
		{
			perror("[!!] Fatal, Creating Thread");
			close(sock_fd);
			close(newsock_fd);
			exit(-1);
		}
	
	}

	close(sock_fd); //
	return 0;
}

void *connect_handle(void *arg)
{
		pthread_detach(pthread_self()); //don't wait for me to return
		int newsock_fd=*((int *)arg);
		int recv_length;
		char buff[1024];
		bzero(buff, sizeof(buff));

		// Talk to socket
		send(newsock_fd, "My first socket!\n",17,0 );
		while ( (recv_length=recv(newsock_fd, &buff, 1024, 0)) > 0)
		{
			printf("RECV: %d bytes\n", recv_length);
			hexdump(buff, recv_length);
		}

		close(newsock_fd);
		return NULL;
}

void hexdump(const unsigned char *buffer, const unsigned int length )
{
	unsigned char byte;
	unsigned int i, j;

	for(i = 0; i < length ; i++)
	{
		byte=buffer[i];
		printf("%02x ", buffer[i]);

		if( (i%16 == 15) || (i == length-1))
		{
			for(j=0; j < 15-(i%16); j++)
				printf(" _ ");
			printf("| ");

			/*display printable bytes*/
			for(j=(i-(i%16)); j<=i; j++)
			{
				byte=buffer[j];
				if( (byte > 31) && (byte < 127))
					printf("%c",byte);
				else
					printf(".");
			}
			
			printf("\n");
		}
	}
}
