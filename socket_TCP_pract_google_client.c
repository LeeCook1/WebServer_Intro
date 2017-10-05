#include <stdio.h>
#include <stdlib.h> //exit
#include <string.h> //strlen
#include "color.h" //my colors
#include <arpa/inet.h> //in_addr
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h> //socket, setsockopt
#include <netdb.h> //struct hostent, gethostbyname

/* Server
 *Create Socket/options
 *Set up Address
 *Bind
 *Listen
 *Accept
 */

/* Client
 *Create Socket/options
 *Setup Destination Address
 *Connect
 *Send Data
 */
#define PORT 80
struct in_addr host_to_ip(const char *hostname)
{
	struct hostent *host_info;
	struct in_addr **addr_list;
	int i;
	char host_ip[100];
	if((host_info = gethostbyname(hostname))==NULL){
		herror(RED"[-] Fatal"RESET"Getting hostname\n");
		exit(-1);
	}

	addr_list= (struct in_addr**)host_info->h_addr_list;

	printf("Address Info\n");
	printf(BLU "\tIP Addresss: "RESET);

	for(i = 0; addr_list[i] != NULL; i++){	
		strncpy(host_ip, inet_ntoa(*addr_list[i]), 100);
		printf("%s ",host_ip);
	}

	printf("\n");

	printf(BLU"\tOfficial Name:"RESET" %s\n",host_info->h_name);
	
	return (*addr_list[i-1]); 
}

int main(int argc, char *argv[])
{
	int sockfd;
	char *msg, serv_reply[2000];
	/*Create Socket*/
	if(	(sockfd=socket(PF_INET, SOCK_STREAM, 0)) == -1){

		perror(RED"[-] Fatal"RESET", creating socket");
		exit(-1);
	}

	/*Set up Dest addr*/
	struct sockaddr_in server; 
	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);
	server.sin_addr=host_to_ip("www.google.com");

	if(connect(sockfd, (struct sockaddr*)&server, sizeof(server)) == -1){
		perror(RED"[-] Fatal"RESET", Connect to Server!");
		error(-1);
	}
	printf(GRN"[+] "RESET "Connection Completed!\n");
	msg="GET / HTTP/1.1\r\n\r\n";

	if(send(sockfd, msg, strlen(msg),0) == -1){
		perror(RED"[-] Fatal"RESET", Send Data");
		exit(-1);
	}
	printf(GRN"[+] "RESET"Message Sent!\n");
	if(recv(sockfd, serv_reply,2000,0) ==-1){
		perror(RED"[-]"RESET"Recv Failed\n");
		exit(-1);
	}
	printf(GRN"[+] "RESET"Received Reply:%s\n",serv_reply);

	close(sockfd);
	return 0;
}
