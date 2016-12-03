/*
 * 
 * My Socket Library
 * 
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

char logStr[2045];
char lastErr[256];
int clientsocket, acceptsocket, serversocket;

char *getLastErr()
{
	return lastErr;
}

void setLastErr(char *err)
{
	strcpy(lastErr,err);
}

int getClientSocket()
{
	return clientsocket;
}

int getServerSocket()
{
	return serversocket;
}

int createClient(int port, char *ip)
{
	struct sockaddr_in addr;

	clientsocket=socket(AF_INET,SOCK_STREAM,0);
	if(clientsocket<0)
	{
		strcpy(lastErr,"Fehler: Der Socket konnte nicht erstellt werden!\n");    
		return 1;
	}		
	printf("ClientSocket erstellt...\n");
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=inet_addr(ip);
	
	printf("Verbindungsaufbau...\n");
	if (connect(clientsocket,(struct sockaddr*)&addr,sizeof(addr))<0)
	{
		strcpy(lastErr,"Fehler: Verbindungsaufbau fehlgeschlagen!\n");
		close(clientsocket);
		return 2;
	}
	printf("Verbunden mit %s:%i\n",ip,port);
	
	return 0;
}

int createServer(int port)
{
	struct sockaddr_in addr;
	
	acceptsocket=socket(AF_INET,SOCK_STREAM,0);	
	if(acceptsocket<0)
	{
		strcpy(lastErr,"Fehler: Der Socket konnte nicht erstellt werden!\n");
		return 1;
	}
	printf("Accept Socket erstellt: %d\n",acceptsocket);
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;
	
	if (bind(acceptsocket,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	{
		strcpy(lastErr,"Bind fehlgeschlagen!\n");
		close(acceptsocket);
		return 2;
	}
	printf("Bind Socket mit Port: %i\n\n",port);
	
	if (listen(acceptsocket,10)<0)
	{
		strcpy(lastErr,"Listen fehlgeschlagen!\n");
		close(acceptsocket);
		return 3;
	}
	
	return 0;
}

void closeSocket(int sock)
{
	close(sock);
}

void closeClient()
{
	close(clientsocket);
}

void closeServer()
{
	close(serversocket);
	close(acceptsocket);
}

int waitForConnections()
{
	struct sockaddr_in client_addr;
	int sz_sock = sizeof(struct sockaddr_in);
	
	serversocket=accept(acceptsocket,(struct sockaddr*)&client_addr,(socklen_t*)&sz_sock);
	if(serversocket<0)
	{
		strcpy(lastErr,"Verbindung fehlgeschlagen!\n");		
		return 1;
	}
	printf("Verbindung akzeptiert. Verbunden mit Serversocket: %d\n",serversocket);
	
	return 0;
}

int ssend(int sock, char *buf, unsigned int sz)
{
  int numbytes;
  
  numbytes=send(sock,buf,sz,0);
  if (numbytes<=0)
  {
    strcpy(lastErr,"Senden fehlgeschlagen!\n");
    return 1;
  }
  printf("%i Bytes gesendet!\n",numbytes);
  return 0;
}

int srcv(int sock, char *buf, unsigned int sz)
{
  int numbytes;
  
  numbytes=recv(sock,buf,sz,0);
  if (numbytes<=0)
  {
    strcpy(lastErr,"Empfangen fehlgeschlagen!\n");    
    return 1;
  }  
  printf("%i Bytes empfangen!\n",numbytes);
  return 0;
}
