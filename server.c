#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <dirent.h>

void checkError(int status)
{
   if (status < 0) {
      printf("socket error(%d): [%s]\n",getpid(),strerror(errno));
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

void sendPeers(int* clients,int nbc,int skip,char* msg)
{
   for(int i=0;i<nbc;i++) {
      if (i==skip) continue;
      int sent=0,rem=strlen(msg);
      while (rem) {
         int nbb = write(clients[i],msg + sent,rem);
         rem -= nbb;
	 sent += nbb;
      }
   }
}
                    
int main(int argc,char* argv[]) 
{
   // Create a socket
   int sid = socket(PF_INET,SOCK_STREAM,0);

   // Make the **server-side** socket non-blocking
   int flags = fcntl(sid, F_GETFL, 0);
   checkError(flags);
   flags = flags | O_NONBLOCK;
   int status = fcntl(sid, F_SETFL, flags);
   checkError(status);
   int enable = 1;
   status = setsockopt(sid, SOL_SOCKET,SO_REUSEADDR, &enable, sizeof(int));
   checkError(status);
  
   // setup our address -- will listen on 8025 --
   struct sockaddr_in addr;
   addr.sin_family = AF_INET;
   addr.sin_port   = htons(8075);
   addr.sin_addr.s_addr = INADDR_ANY;
   //pairs the newly created socket with the requested address.
   status = bind(sid,(struct sockaddr*)&addr,sizeof(addr));
   checkError(status);
   // listen on that socket for "Let's talk" message. No more than 10 pending at once
   status = listen(sid,10);
   checkError(status);
   int  nbc = 0,mxc = 8;   
   int* clients = malloc(sizeof(int)*mxc);
   int goon = 1;
   while(goon) {
      fd_set afd;
      FD_ZERO(&afd);
      FD_SET(sid,&afd);
      int maxFD = sid;
      for(int i=0;i<nbc;i++) {
         FD_SET(clients[i],&afd);
         maxFD = clients[i] > maxFD ? clients[i] : maxFD;
      }
      int nbReady = select(maxFD + 1,&afd,NULL,NULL,NULL);
      //printf("SELECT: %d\n",nbReady);fflush(stdout);            
      if (nbReady > 0) {
         if (FD_ISSET(sid,&afd)) {
            struct sockaddr_in client;
            socklen_t clientSize = sizeof(client);
            //printf("Call accept...\n");fflush(stdout);
            int cliSocket = accept(sid,(struct sockaddr*)&client,&clientSize);
            checkError(cliSocket);
            printf("We accepted a socket: %d\n",cliSocket);
            if (nbc == mxc) {
               mxc = mxc * 2;
               clients = realloc(clients,sizeof(int)*mxc);
            }
            clients[nbc++] = cliSocket;
         }
         for(int i=0;i<nbc;i++) {
            if (FD_ISSET(clients[i],&afd)) {
	       //printf("Client: %d\n",i);
               char* msg = readResponse(clients[i]);
	       //printf("relay: [%s]\n",msg);
               if (strncmp(msg,"$exit",5) == 0)
                  goon = 0;              
               sendPeers(clients,nbc,i,msg);
               free(msg);
            }
         }
      }      
   }
   for(int i=0;i<nbc;i++) 
      close(clients[i]);
   close(sid);
   return 0;
}
