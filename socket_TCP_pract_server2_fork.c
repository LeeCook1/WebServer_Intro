#include <stdio.h>
#include <stdlib.h> //exit
#include <string.h> //memset
#include <sys/socket.h> //socket, setsockopt
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> //inet_ntop

#define PORT 7890
void hexdump(const unsigned char *buffer, const unsigned int length );

/* Server Accept MultiConnections w/ Fork, 
 * Inefficient due to context switching for each call to fork:
 *	page table: maps virtual address to physical
 *	process table: stores vital information
 *	file table: track the proccess's open files
 */

int main(int argc, char *argv[])
{
	int sock_fd,newsock_fd;
	int recv_length=1,optval_yes=1;
	socklen_t sin_size;
	struct sockaddr_in my_addr, client_addr; //address info
	char buff[1024];
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
	while(1)
	{
		/* extract connection from queue, create new fd for connected socket*/
		int pid=-1;
		sin_size= sizeof(struct sockaddr_in);
		if((newsock_fd= accept(sock_fd, (struct sockaddr*)&client_addr, &sin_size))== -1 )
		{
			perror("[!!] Fatal, Accepting connection");
			error(-1);
		}


		if((pid=fork())==-1)
		{
			perror("[!!] Fatal, Forking process");
			exit(-1);
		}
		else if(pid>0)
		{
			// the parent
			close(newsock_fd);
			continue;
		}
		
		// the child
		close(sock_fd); //bind socket
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(client_addr.sin_addr), str, INET_ADDRSTRLEN);
		//Print client info using inet_ntoa, ntoh network to ascii, network to host
		printf("Server: got connection from %s, at port %d\n",str, ntohs(client_addr.sin_port) );

		// Talk to socket
		send(newsock_fd, "My first socket!\n",17,0 );
		while ( (recv_length=recv(newsock_fd, &buff, 1024, 0)) > 0)
		{
			printf("RECV: %d bytes\n", recv_length);
			hexdump(buff, recv_length);
		}

		close(newsock_fd);
		
	}
	
	close(sock_fd);
	return 0;
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
