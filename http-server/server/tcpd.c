#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset()
#include <unistd.h> //close()

#define QUEUELIMIT 5
#define DEFAULT_PORT "8000"
#define DOCUMENT_ROOT "/Users/sunouchimitsuruga/Desktop/tech/C_homework/http-server/server/root"

void http(int sockfd);
int send_msg(int fd, char *msg);

int main(int argc, char* argv[]) {
   int servSock; //server socket descripter
   int clitSock; //client socket descripter
   struct sockaddr_in servSockAddr; //server internet socket address
   struct sockaddr_in clitSockAddr; //client internet socket address
   unsigned short servPort; //server port number
   unsigned int clitLen; // client internet socket address length
   char buf[2048];

   if ((servPort = (unsigned short) atoi(DEFAULT_PORT)) == 0) {
       fprintf(stderr, "invalid port number.\n");
       exit(EXIT_FAILURE);
   }

   if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
       perror("socket() failed.");
       exit(EXIT_FAILURE);
   }

   memset(&servSockAddr, 0, sizeof(servSockAddr));
   servSockAddr.sin_family      = AF_INET;
   servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servSockAddr.sin_port        = htons(servPort);

   if (bind(servSock, (struct sockaddr *) &servSockAddr, sizeof(servSockAddr) ) < 0 ) {
       perror("bind() failed.");
       exit(EXIT_FAILURE);
   }

   if (listen(servSock, QUEUELIMIT) < 0) {
       perror("listen() failed.");
       exit(EXIT_FAILURE);
   }

   // 応答用HTTPメッセージ作成
   memset(buf, 0, sizeof(buf));
   sprintf(buf, "HTTP/1.0 200 OK\r\nContent-Length: 20\r\nContent-Type: text/html\r\n\r\nHELLO\r\n");

   while(1) {
       clitLen = sizeof(clitSockAddr);
       clitSock = accept(servSock, (struct sockaddr *) &clitSockAddr, &clitLen);
       if (clitSock < 0) {
           perror("accept() failed.");
           exit(EXIT_FAILURE);
       }
       printf("connected from %s.\n", inet_ntoa(clitSockAddr.sin_addr));
       //send(clitSock, buf, (int)strlen(buf), 0);
       http(clitSock);
       close(clitSock);
   }

    return EXIT_SUCCESS;
}

void http(int sockfd) {
    int len;
    int contentLen;
    int bufsize = 2048;
    char buf[bufsize];
    char fbuf[bufsize];
    char meth_name[16];
    char url_addr[256];
    char http_ver[64];
    char path[256];
    char url_file[2048];
    char *header = "HTTP/1.0 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n";
    char *httpCode;
    httpCode = "200 OK";
    char msg[2048];
    FILE* read_fd;

    printf("start.\n");
    if (read(sockfd, buf, 1024) <= 0 ) {
        fprintf(stderr, "error: reading a request.\n");
    }else{
        sscanf(buf, "%s %s %s", meth_name, url_addr, http_ver);
        if (strcmp(meth_name, "GET") != 0) {
            httpCode = "501 Not Implemented";
            printf("501.\n");
        }else{
            sprintf(path, "%s%s", DOCUMENT_ROOT, url_addr);
            printf("%s\n", path);
            if ((read_fd = fopen(path, "r")) == NULL) {
                httpCode = "404 Not Found";
                printf("404.\n");
            }else{
                sprintf(url_file, "");
	            while(fgets(fbuf, bufsize, read_fd) != NULL) {
		              sprintf(url_file, "%s%s\r\n", url_file, fbuf);
	            }
                contentLen = strlen(url_file);
                sprintf(msg, header, httpCode, "text/html", contentLen);
        	    send_msg(sockfd, msg);
                send_msg(sockfd, url_file);

        	    fclose(read_fd);
            }
        }
    }
}

int send_msg(int fd, char *msg) {
    int len;
    len = strlen(msg);
    if ( write(fd, msg, len) != len ){
        fprintf(stderr, "error: writing.");
    }
    return len;
}
