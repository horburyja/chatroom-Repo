#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <assert.h>

void checkError(int status,int line)
{
   if (status < 0) {
      printf("socket error(%d)-%d: [%s]\n",getpid(),line,strerror(errno));
      exit(-1);
   }
}

char* readResponse(int fd) {
   int sz = 8;
   char* buf = malloc(sz);
   int ttl = 0,at = 0;
   int recvd;
   do {
      recvd = read(fd,buf+at,1);
      ttl += recvd;
      at  += recvd;
      if (recvd > 0 && ttl == sz) {
         buf = realloc(buf,sz*2);
         sz *= 2;
      }
      buf[ttl] = 0;
   } while (buf[ttl-1] != '\n' && recvd > 0);
   return buf;
}

int main(int argc,char* argv[]) 
{
   // Create a socket
   if (argc < 2) {
      printf("Usage is: client <hostname>\n");
      return 1;
   }
   int sid = socket(PF_INET,SOCK_STREAM,0);
   struct sockaddr_in srv;
   struct hostent *server = gethostbyname(argv[1]);
   if (server==NULL) {
      printf("Couldn't find a host named: %s\n",argv[1]);
      return 2;
   }
   srv.sin_family = AF_INET;
   srv.sin_port   = htons(8075);
   memcpy(&srv.sin_addr,server->h_addr_list[0],server->h_length);
   int status = connect(sid,(struct sockaddr*)&srv,sizeof(srv));
   checkError(status,__LINE__);

   fd_set afd;
   size_t sz = 512;
   char* buf = malloc(sz);
   memset(buf,0,sz);
   int done = 0;
   do {
     FD_ZERO(&afd);
     FD_SET(STDIN_FILENO,&afd);
     FD_SET(sid,&afd);
     int nbReady = select(sid+1,&afd,NULL,NULL,NULL);
     if (nbReady > 0) {        
       if (FD_ISSET(STDIN_FILENO,&afd)) {
	  int rc = getline(&buf,&sz,stdin);
	  //printf("sending [%s] %d\n",buf,rc);
          int wc = write(sid,buf,rc);
       } else if (FD_ISSET(sid,&afd)) {
 	  char* line = readResponse(sid);
	  //printf("[%s]\n",line);
	  int rc = strlen(line);
	  strcpy(buf,line);
	  free(line);
          done = rc == 0;
          if (!done) {
             int wc = write(STDOUT_FILENO,buf,rc);
             assert(wc > 0);
          }
       }
       done |= strncmp(buf,"$exit",5)==0;             
     } else done = 1;
   } while(!done);
   close(sid);
   return 0;
}

