#include <stdio.h>
#include <string.h>
#include "socket_comm.h"

int sendCmdToServer(char *sendbuf)
{
	int status;
	int sock;
	
	sock = getClientSocket();
	status = ssend(sock,sendbuf,32);
	if(status)
	{
		setLastErr("Verbindung unterbrochen!\n");		
		closeSocket(sock);
		return 1;
	}
	return 0;
}

int rcvCmdFromClient()
{	
	int status;
	int sock;
	char rcvbuf[32];
	
	sock = getServerSocket();
	status = srcv(sock,rcvbuf,32);
	if(status)
	{
		setLastErr("Verbindung unterbrochen!\n");		
		closeSocket(sock);
		return 1;
	}
	printf("Command: %s\n",rcvbuf);
	
	return 0;
}
