#ifndef SOCKET_COMM_H
#define SOCKET_COMM_H

#define CMD_OPEN_FILE "open_file"
#define CMD_OPEN_FILE_MSD "msd_open_file"
#define CMD_READ_FILE "read_file"
#define CMD_WRITE_FILE "write_file"
#define CMD_WRITE_FILE_MSD "msd_write_file"
#define CMD_GET_FILESIZE "get_filesize"
#define CMD_CLOSE_FILE "close_file"
#define CMD_CLOSE_FILE_MSD "msd_close_file"

#define NO_FILE "noFile"
#define FILE_OPEN "file_open"
#define FILE_CLOSED "file_closed"
#define INPUT_OK "in_ok"
#define WRITE_OK "write_ok"
#define INPUT_FAILED "in_failed"

typedef enum {ok,err1,err2,err3,noConn,noFile,inFailed,msdInitFailed,msdWriteFailed} status;

char *getLastErr();
void setLastErr(char *err);
int getClientSocket();
int getServerSocket();
status createClient(int port, char *ip);
status createServer(int port);
void closeSocket(int sock);
void closeClient();
void closeServer();
status waitForConnections();
status ssend(int sock, char *buf, unsigned int sz);
status srcv(int sock, char *buf, unsigned int sz);
status sendCmdToServer(void **plhs, int nrhs, void *prhs[]);
status rcvCmdFromClient();
unsigned int getFilesize();
status openFile(char *filename, char *rwflag);
status openFileMSD(char *filename, char *rwflag);
status readFile(char *buf, unsigned int sz);
status writeFile(char *buf, unsigned int sz);
status writeFileMSD(char *buf, unsigned int sz);
status closeFile();
status closeFileMSD();

#endif
