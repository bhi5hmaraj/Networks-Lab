  /* CSD 304 Computer Networks, Fall 2016
     Lab 4, multicast receiver
     Team: Bhishmaraj , Kishore , Aditya , Akhil
  */

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <net/if.h>
  #include <netdb.h>
  #include <sys/ioctl.h>
  #include <signal.h>

  #define MC_PORT 5432
  #define TCP_PORT 5402
  #define BUF_SIZE 40000
  #define MAX_LINE 4096

     int main(int argc, char * argv[]){
      struct hostent *hp;
      struct sockaddr_in sin1;
      char *host;
      char tcp_buf[MAX_LINE];
      int tcp_sock;
      host = "0.0.0.0\0";
      hp = gethostbyname(host);
      if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
      }
      else
        printf("Client's remote host: %s\n", argv[1]);
    /* build address data structure */
      bzero((char *)&sin1, sizeof(sin1));
      sin1.sin_family = AF_INET;
      bcopy(hp->h_addr, (char *)&sin1.sin_addr, hp->h_length);
      sin1.sin_port = htons(TCP_PORT);
    /* active open */
      if ((tcp_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
      }
      else
        printf("Client created socket.\n");

      if (connect(tcp_sock, (struct sockaddr *)&sin1, sizeof(sin1)) < 0)
      {
        perror("simplex-talk: connect");
        close(tcp_sock);
        exit(1);
      }
      else
        printf("Client connected.\n");
    /* main loop: get and send lines of text */
      memset(tcp_buf , 0 , sizeof(tcp_buf));
      recv(tcp_sock, tcp_buf, sizeof(tcp_buf), 0);
      fputs(tcp_buf,stdout);
      fflush(stdout);

      int udp_mc_sock; /* socket descriptor */
      struct sockaddr_in sin2; /* socket struct */
      char *if_name; /* name of interface */
      struct ifreq ifr; /* interface struct */
      char mc_buf[BUF_SIZE];
      /* Multicast specific */
      char *mcast_addr; /* multicast address */
      struct ip_mreq mcast_req;  /* multicast join struct */
      struct sockaddr_in mcast_saddr; /* multicast sender*/
      socklen_t mcast_saddr_len;
      mcast_addr = "230.192.1.10";
    /* Add code to take port number from user */
/*      if ((argc==2)||(argc == 3)) {
        mcast_addr = argv[1];
      }
      else {
        fprintf(stderr, "usage:(sudo) receiver multicast_address [interface_name (optional)]\n");
        exit(1);
      }*/

        if(argc == 3) {
          if_name = argv[2];
        }
        else
          if_name = "wlan0";


    /* create socket */
        if ((udp_mc_sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
          perror("receiver: socket");
          exit(1);
        }

    /* build address data structure */
        memset((char *)&sin2, 0, sizeof(sin2));
        sin2.sin_family = AF_INET;
        sin2.sin_addr.s_addr = htonl(INADDR_ANY);
        sin2.sin_port = htons(MC_PORT);


    /*Use the interface specified */ 
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name , if_name, sizeof(if_name)-1);

        if ((setsockopt(udp_mc_sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, 
          sizeof(ifr))) < 0)
        {
          perror("receiver: setsockopt() error");
          close(udp_mc_sock);
          exit(1);
        }

    /* bind the socket */
        if ((bind(udp_mc_sock, (struct sockaddr *) &sin2, sizeof(sin2))) < 0) {
          perror("receiver: bind()");
          exit(1);
        }

    /* Multicast specific code follows */

    /* build IGMP join message structure */
        mcast_req.imr_multiaddr.s_addr = inet_addr(mcast_addr);
        mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);

    /* send multicast join message */
        if ((setsockopt(udp_mc_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
          (void*) &mcast_req, sizeof(mcast_req))) < 0) {
          perror("mcast join receive: setsockopt()");
        exit(1);
      }



    /* receive multicast messages */  
      printf("\nReady to listen !\n\n");

      int pid;
    if((pid = fork()) == 0) { // This child process will be recieving song packets
      FILE* fptr = fopen("dump.ts" , "wb");
      int packet = 1;
      int len;
      while(1) {

          /* reset sender struct */
        memset(&mcast_saddr, 0, sizeof(mcast_saddr));
        mcast_saddr_len = sizeof(mcast_saddr);

          /* clear buffer and receive */
        memset(mc_buf, 0, sizeof(mc_buf));
        if ((len = recvfrom(udp_mc_sock, mc_buf, BUF_SIZE, 0, (struct sockaddr*)&mcast_saddr, 
         &mcast_saddr_len)) < 0) {
          perror("receiver: recvfrom()");
        exit(1);
      }
      printf("Recieved Packet = %d\n", (packet++));
      fflush(stdout);
      int i;
      for (i = 0; i < len; ++i)
        fputc(mc_buf[i] , fptr);

      fflush(fptr);
          /* Add code to send multicast leave request */

      /*
      Pipe to vlc
      ffmpeg -i dump.ts -f mp3 -| cvlc -
      */

    }
  }
  else {
    while(1) {
      printf("P - Pause , R - Resume , C - Station List (Unsupported) , X - Terminate\n");
      char choice = getchar();
        if(choice == 'P') // Switch case has some controvorsies 
          kill(pid , SIGSTOP);
        else if(choice == 'R')
          kill(pid , SIGCONT);
        else if(choice == 'X') {
          printf("process terminated\n");
          fflush(stdout);
          kill(pid , SIGTERM);
          close(udp_mc_sock);
          close(tcp_sock);
          return 0;
        }
        else {
          printf("Invalid Operation\n");
          fflush(stdout);
        }
      }
    }    
  }