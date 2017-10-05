#include <stdio.h> // printf, perror
#include <stdlib.h> //exit
#include <string.h> //bzero, memset
#include <sys/socket.h> //socket
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h> //inet_ntop

#include "color.h"
#include "my_funcs.h"

#define UDP_PORT 7890
#define BUFF_LEN 1024
/* UDP Server
 * Socket
 * Bind
 * Recvfrom/Sendto
*/
void do_work(int sockfd, struct sockaddr* client, char *buffer,int len);
int main(int argc, char *argv[])
{
	char buff[BUFF_LEN], addr_str[INET_ADDRSTRLEN];
	struct sockaddr_in host_addr, client_addr;
	int sockfd, recv_len, addr_len= sizeof(struct sockaddr_in);

	if((sockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1) // Ipv4, UDP Socket, default protocol
		error_m("Socket");

	memset(&host_addr,'\0', sizeof(struct sockaddr_in));

	host_addr = (struct sockaddr_in){ AF_INET, htons(UDP_PORT), {0} }; // sin_family,sin_port,sin_addr.s_addr

	if(bind(sockfd, (struct sockaddr*)&host_addr, sizeof(struct sockaddr))==-1)
	error_m("Binding");

	while( (recv_len=recvfrom(sockfd,buff,BUFF_LEN-1,0,(struct sockaddr*)&client_addr,(struct socklen_t*)&addr_len)) > 0 && inet_ntop(AF_INET,&(client_addr.sin_addr), addr_str,INET_ADDRSTRLEN ) )
	{
		buff[BUFF_LEN]='\0';
		printf("Packet from: %s on port %d\n",addr_str, ntohs(client_addr.sin_port) );

		printf("Data: %s", buff);
		do_work(sockfd,(struct sockaddr*)&client_addr,(char *)&buff,recv_len);
		memset(buff,'\0',BUFF_LEN);
	}

	if(recv_len == -1)
		error_m("Receive");

	close(sockfd);
	return 0;
}


void do_work(int sockfd, struct sockaddr* client, char *buffer,int len)
{
	
	if(sendto(sockfd, buffer, len,0,client, sizeof(struct sockaddr))==-1)
		error_m("Send");
	
}
