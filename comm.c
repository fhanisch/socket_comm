#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msd.h>
#include "socket_comm.h"

FILE *file=NULL;
static char *data=NULL;
static unsigned int dataPtr;
static libusb_device_handle *handle = NULL;

status sendCmdToServer(void **plhs, int nrhs, void *prhs[])
{
	status st;
	int sock;
	char *rcvbuf=malloc(256);
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
	char filename[32];	
	char rwFlag[1];
	unsigned int bufsize;
	unsigned int filesize;
	uint8_t buffer[1024];
	char vid[9], pid[9], rev[5];
	int i;
	uint32_t max_lba, block_size;
	double device_size;
		
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
	else if (!strcmp(rcvbuf,CMD_OPEN_FILE_MSD))
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
		
		st = msd_init(&handle);
		if (st)
		{
			printf("msd init failed!\n");
			st = ssend(sock,INPUT_FAILED,32);
			if(st)
			{
				setLastErr("Verbindung unterbrochen!\n");		
				closeSocket(sock);
				return noConn;
			}
			return msdInitFailed;
		}
		printf("MSD init ok!\n\n");
		
		//Inquiry
		memset(buffer, 0, sizeof(buffer));
		st = msd_inquiry(handle, buffer);
		if (st)
		{
			printf("inquiry failed!\n");
			st = ssend(sock,INPUT_FAILED,32);
			if(st)
			{
				setLastErr("Verbindung unterbrochen!\n");		
				closeSocket(sock);
				return noConn;
			}
			return msdInitFailed;
		}
		// The following strings are not zero terminated
		for (i=0; i<8; i++) {
			vid[i] = buffer[8+i];
			pid[i] = buffer[16+i];
			rev[i/2] = buffer[32+i/2];	// instead of another loop
		}
		vid[8] = 0;
		pid[8] = 0;
		rev[4] = 0;
		printf("   VID:PID:REV \"%8s\":\"%8s\":\"%4s\"\n", vid, pid, rev);
		
		// Read capacity
		memset(buffer, 0, sizeof(buffer));
		st = msd_read_capacity(handle, buffer);
		if (st)
		{
			printf("Read capacity failed!\n");
			st = ssend(sock,INPUT_FAILED,32);
			if(st)
			{
				setLastErr("Verbindung unterbrochen!\n");		
				closeSocket(sock);
				return noConn;
			}
			return msdInitFailed;
		}
		max_lba = be_to_int32(&buffer[0]);
		block_size = be_to_int32(&buffer[4]);
		device_size = ((double)(max_lba+1))*block_size/(1024*1024*1024);
		printf("   Max LBA: %08X, Block Size: %08X (%.2f GB)\n", max_lba, block_size, device_size);
		
		if (rwFlag[0]=='r')
		{
			
		}
		else if (rwFlag[0]=='w')
		{
			data = malloc(1024);
			dataPtr = 0;
			memset(data, 0, 1024);
			strcpy(data,filename);
			dataPtr+=strlen(filename)+1;																							
		}
		else
		{
			printf("Unbekanntes rwFlag: %s\n",rwFlag);
			st = ssend(sock,INPUT_FAILED,32);
			if(st)
			{
				setLastErr("Verbindung unterbrochen!\n");		
				closeSocket(sock);
				return noConn;
			}
			return msdInitFailed;
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
		bufsize = *(unsigned int*)rcvbuf;		
		
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
		
		fwrite(rcvbuf,1,bufsize,file);
		
		st = ssend(sock,WRITE_OK,32);
		if(st)
		{
			setLastErr("Verbindung unterbrochen!\n");		
			closeSocket(sock);
			return noConn;
		}
		
	}
	else if (!strcmp(rcvbuf,CMD_WRITE_FILE_MSD))
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
		bufsize = *(unsigned int*)rcvbuf;		
		
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
				
		memcpy(data+dataPtr,rcvbuf,bufsize);
		dataPtr+=bufsize;
		
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
	else if (!strcmp(rcvbuf,CMD_CLOSE_FILE_MSD))
	{				
		st = msd_write(handle, (uint8_t*)data, 0, 1024);
		if(st)
		{
			printf("Write File failed!\n");
			st = ssend(sock,INPUT_FAILED,32);
			if(st)
			{
				setLastErr("Verbindung unterbrochen!\n");		
				closeSocket(sock);
				return noConn;
			}
			return msdWriteFailed;
		}
		msd_close_dev(handle);
		free(data);
		
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

status openFile(char *filename, char *rwflag)
{
	status st;
	void *prhs[3];
	void *plhs;
	
	prhs[0] = CMD_OPEN_FILE;
	prhs[1] = filename;
	prhs[2] = rwflag;
	st = sendCmdToServer(&plhs, 3, prhs);
	if(st) return inFailed;
	if (!strcmp(plhs,NO_FILE))
	{
		free(plhs);
		return noFile;
	}
	
	return ok;
}

status openFileMSD(char *filename, char *rwflag)
{
	status st;
	void *prhs[3];
	void *plhs;
	
	prhs[0] = CMD_OPEN_FILE_MSD;
	prhs[1] = filename;
	prhs[2] = rwflag;
	st = sendCmdToServer(&plhs, 3, prhs);
	if(st) return inFailed;
	if (!strcmp(plhs,NO_FILE))
	{
		free(plhs);
		return noFile;
	}
	
	return ok;
}

unsigned int getFilesize()
{	
	unsigned int filesize;
	void *prhs[1];
	void *plhs;
	
	prhs[0] = CMD_GET_FILESIZE;
	sendCmdToServer(&plhs,1,prhs);
	filesize = *(unsigned int*)plhs;
	free(plhs);
	
	return filesize;
}

status readFile(char *buf, unsigned int sz)
{
	status st;
	void *prhs[2];
	void *plhs;
	
	prhs[0] = CMD_READ_FILE;	
	prhs[1] = &sz;	
	st = sendCmdToServer(&plhs,2,prhs);	
	memcpy(buf,plhs,sz);
	free(plhs);
	
	return st;
}

status writeFile(char *buf, unsigned int sz)
{
	status st;
	void *prhs[3];
	void *plhs;
	
	prhs[0] = CMD_WRITE_FILE;
	prhs[1] = &sz;
	prhs[2] = buf;	
	st = sendCmdToServer(&plhs, 3, prhs);	
	free(plhs);
	
	return st;
}

status writeFileMSD(char *buf, unsigned int sz)
{
	status st;
	void *prhs[3];
	void *plhs;
	
	prhs[0] = CMD_WRITE_FILE_MSD;
	prhs[1] = &sz;
	prhs[2] = buf;	
	st = sendCmdToServer(&plhs, 3, prhs);	
	free(plhs);
	
	return st;
}

status closeFile()
{
	status st;
	void *prhs[1];
	void *plhs;
	
	prhs[0] = CMD_CLOSE_FILE;
	st = sendCmdToServer(&plhs,1,prhs);
	free(plhs);
	
	return st;
}

status closeFileMSD()
{
	status st;
	void *prhs[1];
	void *plhs;
	
	prhs[0] = CMD_CLOSE_FILE_MSD;
	st = sendCmdToServer(&plhs,1,prhs);
	free(plhs);
	
	return st;
}
