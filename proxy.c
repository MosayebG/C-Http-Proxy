#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>


int main(int argc, char * argv[])
{	
	
	if (argc != 3)
	{
	fprintf(stderr, "usage:  ./proxy <ip> <port>\n\nip: your system ip\nport: 1024-65530\n");
	exit(1);
	}
	
	int serverSocket;
	int port;
	char * serverip;
	char filterPage[] = "HTTP/1.1 302 Found\n\n<html><head><title>this web page is filter !..!</title></head><body><h1>This page in filter</h1><p><h3>Please Leave This site.</h3></p></body></html>";
	int clientSocket;
	int size;
	int i;
	int clientPort;
	char * filterSites = "";
	port = atoi(argv[2]);
	serverip = argv[1];
	char clientip[32];
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	size = sizeof(clientAddress);
		
	if ((serverSocket = socket(AF_INET, SOCK_STREAM,0)) < 0)
	{
		fprintf(stderr, "\nsocket can't be created....try again! ");
		exit(1);	
	}
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr(serverip);
	
	if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		fprintf(stderr, "\naddress can't bind to the socket....try again! ");
		exit(1);
	}
	listen(serverSocket, 10);
	
	while(1)
	{
		char buffer[1024];
		char fakebuf[1024];
		char * firstline;
		char * secondline;
		char * site;
		char * host;
		char data[250];
		char * isvalid;
		char response[100000] = {0};
		int i;
		int middleSocket;
		struct sockaddr_in middleAddress;
		struct hostent *siteIp;
		struct in_addr **siteAddress;
		
		printf("\nProxy Waiting for request ...\n\n");	
		
		if((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &size)) < 0)
		{
			fprintf(stderr,"\n the socket can't accept the connection....try again! ");
			exit(1);
		}
		inet_ntop(AF_INET, &(clientAddress.sin_addr), clientip, 32);
		
		printf("\nClient with address:%s now is connect....\n",clientip);
		
		if(recv(clientSocket, buffer, sizeof(buffer), 0) < 0)
		{
			fprintf(stderr, "request resiving error! ");
			exit(1);
		}
		
		strcpy(fakebuf, buffer);
		firstline = strtok(fakebuf, "\r\n");
		secondline = strtok(NULL, "\r\n");
		host = strtok(secondline, ": ");
		site = strtok(NULL, ": ");
		
		printf("\nthe client want to visit the %s website.\n",site);
		
		isvalid = strstr(filterSites, site);
		
		if(isvalid != NULL)
		{	
			printf("\nthis website is filter. filterpage sent to the client.\n");
			
			if( send(clientSocket, filterPage, sizeof(filterPage), 0) < 0)
			{
				fprintf(stderr, "sending filterpage to client error!! ");
				exit(1);
			}
			
			close(clientSocket);
		}
		else
		{	
			
			middleSocket = socket(AF_INET, SOCK_STREAM, 0);
			
			siteIp = gethostbyname(site);
			siteAddress = (struct in_addr **)siteIp->h_addr_list;
			
			middleAddress.sin_family = AF_INET;
			middleAddress.sin_port = htons(80);
			middleAddress.sin_addr.s_addr = inet_addr(inet_ntoa(*siteAddress[0]));
			
			if (connect(middleSocket, (struct sockaddr *) &middleAddress, sizeof(middleAddress)) < 0)
			{
				fprintf(stderr,"Error in connecting to web server! ");
				exit(1);
			}
		  	
			if (send(middleSocket, buffer, strlen(buffer),0) < 0)
			{
				fprintf(stderr, "error in sending client request to the web server! ");
				exit(1);
			}
			
			while(1)
			{
				
				ssize_t datalen = recv(middleSocket, data, sizeof(data)-1, 0);
					
				if (datalen > 0)
				{	
					data[datalen] = '\0';
					strcat(response, data);
				}
				else if (datalen == 0)
				{
					break;
				}
				else
				{
					fprintf(stderr, "error in reciving response fron the webserver! ");
                                        exit(1);
				}
				
			}
			if (send(clientSocket, response, strlen(response),0) < 0)
			{
				fprintf(stderr, "error in sending the response to the client! ");
				exit(1);
			}
			
			close(clientSocket);
				
		}
				
		for(i=1; i<3; i++)
                {
                        printf("#######################################################\n");
                }

	}
	return 0;
}
