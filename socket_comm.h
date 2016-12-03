#ifndef SOCKET_COMM_H
#define SOCKET_COMM_H

#define CMD_OPEN_FILE "open_file"
#define CMD_READ_FILE "read_file"

char *getLastErr();
void setLastErr(char *err);
int getClientSocket();
int getServerSocket();
int createClient(int port, char *ip);
int createServer(int port);
void closeSocket(int sock);
void closeClient();
void closeServer();
int waitForConnections();
int ssend(int sock, char *buf, unsigned int sz);
int srcv(int sock, char *buf, unsigned int sz);
int sendCmdToServer(char *sendbuf);
int rcvCmdFromClient();

#endif
