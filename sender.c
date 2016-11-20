/* CSD 304 Computer Networks, Fall 2016
   Lab 4, Sender
   Team: Bhishmaraj , Kishore , Aditya , Akhil
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MC_PORT 5432
#define TCP_PORT 5402
#define BUF_SIZE 32000
#define MAX_PENDING 10

   uint32_t IPAddressToInt(char* str) {
    int digit[4];
    uint32_t ans = 0;
    sscanf(str , "%d.%d.%d.%d" , digit , digit + 1 , digit + 2 , digit + 3);
    int i;
    for(i=0;i<4;i++) {
      ans |= digit[i];
      ans <<= 8;
    }
    return ans;
  }

  int main(int argc, char * argv[]){
    /* Multicast specific */
    char *mcast_addr; /* multicast address */
    mcast_addr = "230.192.1.10\0";

  if(fork() == 0)  {  // The child process will be creating the multicast group and send the song packets
    
    printf("Child process created\n");
    fflush(stdout);
    int s; /* socket descriptor */
    struct sockaddr_in sin; /* socket struct */
    char buf[BUF_SIZE];
    int len;




  /*TODO Add code to take port number from user */
/*    if (argc==2) {
      mcast_addr = argv[1];
    }
    else {
      fprintf(stderr, "usage: sender multicast_address\n");
      exit(1);
    }*/

  /* Create a socket */
      if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server: socket");
        exit(1);
      }

  /* build address data structure */
      memset((char *)&sin, 0, sizeof(sin));
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = inet_addr(mcast_addr);
      sin.sin_port = htons(MC_PORT);


  /* Send multicast messages */
  /* Warning: This implementation sends strings ONLY */
  /* TODO You need to change it for sending A/V files */
      FILE* fptr = fopen("./data/a.ts" , "rb");
      if(fptr != NULL)
        printf("File opened sucessfully\n");
      else
        printf("File not opened\n");
      fflush(stdout);
      fseek(fptr, 0, SEEK_END);
      int length = ftell(fptr);
      fseek(fptr, 0, SEEK_SET);
      int remain = length;
      int packet = 1;
      while(remain > 0) {
        int ptr;
        memset(buf, 0, BUF_SIZE);   
        int to = BUF_SIZE < remain ? BUF_SIZE : remain;
        for(ptr=0;ptr<to;ptr++)
          buf[ptr] = fgetc(fptr);

        remain -= to;
        sendto(s, buf, BUF_SIZE, 0,(struct sockaddr *)&sin, sizeof(sin));
        printf("Sent Packet = %d\n", (packet++));
        fflush(stdout);
        usleep(100000);
      }
      fclose(fptr);
      close(s);
      return 0;
    }
else { // Parent process will be serving the station information to the incomming clients
  struct sockaddr_in sin;
  /*char buf[MAX_LINE];*/
  int len;
  int s, new_s;
  char str[INET_ADDRSTRLEN];

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(TCP_PORT);
  /* setup passive open */
  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
  printf("Server is using address %s and port %d.\n", str, TCP_PORT);

  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind");
    exit(1);
  }
  else
    printf("Server bind done.\n");

  listen(s, MAX_PENDING);
  /* wait for connection, then receive and print text */
  while(1) {

    if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
      perror("simplex-talk: accept");
      exit(1);
    }
    printf("Server Listening.\n");

    char * message = malloc(200);
    int bitrate = 320; // TODO : automatically find bitrate
    sprintf(message , "Hello\nSite Name : SNU Internet Radio\nSite Description : NULL\nStation Count : 1\nStation Number : 1\nStation Name : Hello FM\nMulticast address : %s\nMulticast UDP Port : %d\nInfo Port : %d\nBitrate :%d\n ", mcast_addr , MC_PORT , 0 , bitrate);
    send(new_s,message,strlen(message),0);

  }
}
return 0;

}

