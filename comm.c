#include <stdio.h>
#include <string.h>
#include "socket_comm.h"

FILE *file=NULL;

status sendCmdToServer(void **plhs, int nrhs, void *prhs[])
{
	status st;
	int sock;
	char rcvbuf[256];
	unsigned int i;
	
	sock = getClientSocket();
	for (i=0;i<nrhs;i++)
	{
		st = ssend(sock,prhs[i],32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
	
		st = srcv(sock,rcvbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}				
		if (!strcmp(rcvbuf,INPUT_FAILED))
		{
			printf("Status: %s\n",rcvbuf);
			return inFailed;
		}
	}
	*plhs = rcvbuf;
	
	return ok;
}

status rcvCmdFromClient()
{	
	status st;
	int sock;
	char rcvbuf[256];
	char sendbuf[256];
	char writebuf[256];
	char filename[32];	
	char rwFlag[1];
	unsigned int filesize;
		
	sock = getServerSocket();
	printf("...\n");
	st = srcv(sock,rcvbuf,32);	
	if(st)
	{
		setLastErr("Verbindung unterbrochen!\n");		
		closeSocket(sock);
		return noConn;
	}
	printf("Command: %s\n",rcvbuf);
	
	if (!strcmp(rcvbuf,CMD_OPEN_FILE))
	{
		st = ssend(sock,INPUT_OK,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
		st = srcv(sock,rcvbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		strcpy(filename,rcvbuf);
		printf("Filename: %s\n",filename);
		
		st = ssend(sock,INPUT_OK,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
		st = srcv(sock,rcvbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		strcpy(rwFlag,rcvbuf);
		printf("Read/Write Flag: %s\n",rwFlag);
		
		file = fopen(filename,rwFlag);
		if (!file) 
		{
			st = ssend(sock,NO_FILE,32);
			if(st)
			{
				setLastErr("Verbindung unterbrochen!\n");		
				closeSocket(sock);
				return noConn;
			}
			return noFile;
		}
		st = ssend(sock,FILE_OPEN,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
	}
	else if (!strcmp(rcvbuf,CMD_GET_FILESIZE))
	{		
		fseek(file,0,SEEK_END);
		filesize=ftell(file);
		rewind(file);		
		memcpy(sendbuf,&filesize,sizeof(unsigned int));
		st = ssend(sock,sendbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}		
	}
	else if (!strcmp(rcvbuf,CMD_READ_FILE))
	{
		st = ssend(sock,INPUT_OK,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
		st = srcv(sock,rcvbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		//printf("Bytes to read: %d\n",*(unsigned int*)rcvbuf);
		
		fread(sendbuf,1,*(unsigned int*)rcvbuf,file);
		
		st = ssend(sock,sendbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}	
	}
	else if (!strcmp(rcvbuf,CMD_WRITE_FILE))
	{
		st = ssend(sock,INPUT_OK,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
		st = srcv(sock,rcvbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		strcpy(writebuf,rcvbuf);
		printf("Write: %s\n",writebuf);
		
		st = ssend(sock,INPUT_OK,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
		st = srcv(sock,rcvbuf,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
		fwrite(writebuf,1,*(unsigned int*)rcvbuf,file);
		
		st = ssend(sock,WRITE_OK,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
	}
	else if (!strcmp(rcvbuf,CMD_CLOSE_FILE))
	{		
		fclose(file);
		printf("File geschlossen.\n");
		st = ssend(sock,FILE_CLOSED,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}	
	}
	else
	{
		st = ssend(sock,INPUT_FAILED,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}				
	}
	
	return ok;
}
