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
  #include <fcntl.h>

  // #define MC_PORT 5437
  #define TCP_PORT 5451
  #define BUF_SIZE 15000
  #define MAX_LINE 4096
  #define BUF_TIME 3
  // char BIG_BUF[BUF_SIZE * 10];
     int udp_mc_sock; /* socket descriptor */
     char mc_buf[BUF_SIZE];
     /* Multicast specific */
     
     struct sockaddr_in mcast_saddr; /* multicast sender*/
     socklen_t mcast_saddr_len;

     char* intToIPAddress(uint32_t IP) {
      uint8_t* ptr = (uint8_t*) &IP;
      char* str = malloc(36); // 32 + 3 + 1
      sprintf(str , "%u.%u.%u.%u" ,  *(ptr + 3), *(ptr + 2) , *(ptr + 1) ,*ptr );
      str[35] = '\0';
      return str;
  }

  uint8_t parseByte(char* data) {
     uint8_t bite = data[0];
     return bite;
 }
 uint16_t parseShort(char* data) {
     uint16_t shot = 0;    
     shot |= ((uint8_t)(data[0]));
     shot <<= 8;    
     shot |= ((uint8_t)(data[1]));            
     return shot;
 }

 uint32_t parseInt(char* data) {
     uint32_t Int = 0;
     Int |= ((uint8_t)data[0]);
     Int <<= 8;
     Int |= ((uint8_t)data[1]);
     Int <<= 8;
     Int |= ((uint8_t)data[2]);
     Int <<= 8;
     Int |= ((uint8_t)data[3]);
     return Int;
 }
 char* shortToString(uint16_t num) {
     uint16_t mask = 0xFF;
     char* str = malloc(2);
     str[0] = (num >> 8) & mask;
     str[1] = num & mask;
     fflush(stdout);
     return str; 
 }
 char* intToString(uint32_t num) {
     uint32_t mask = 0xFF;
     char* str = malloc(4);
     str[0] = (num >> 24) & mask;
     str[1] = (num >> 16) & mask;
     str[2] = (num >> 8) & mask;
     str[3] = num & mask;
     return str; 
 }

 typedef struct {
    uint8_t  station_number;
    uint8_t  station_name_size;
    char*    station_name;
    uint32_t multicast_address;
    uint16_t data_port;
    uint16_t info_port;
    uint32_t bit_rate;
}station_info;

typedef struct {
   uint8_t type;
   uint8_t site_name_size;
   char*   site_name;
   uint8_t site_desc_size;
   char*   site_desc;
   uint8_t station_count;
   station_info** station_list;
}site_info;

site_info* deserialize(char* ser) {

  site_info* si = malloc(sizeof(site_info));
  si->type = *(ser++);
  si->site_name_size = *(ser++);
  si->site_name = ser; ser += si->site_name_size;
  si->site_desc_size = *(ser++);
  si->site_desc = ser; ser += si->site_desc_size;
  si->station_count = *(ser++);
  int len = si->station_count;
  int i;
  si->station_list = (station_info**) malloc(sizeof(station_info*) * len);
  for(i=0;i<len;i++) {
    si->station_list[i] = (station_info*) malloc(sizeof(station_info));
    (si->station_list[i])->station_number = *(ser++);
    (si->station_list[i])->station_name_size = *(ser++);
    (si->station_list[i])->station_name = ser; ser += (si->station_list[i])->station_name_size;
    (si->station_list[i])->multicast_address = parseInt(ser); ser += 4;
    (si->station_list[i])->data_port = parseShort(ser); ser += 2;
    (si->station_list[i])->info_port = parseShort(ser); ser += 2;
    (si->station_list[i])->bit_rate  = parseInt(ser); ser += 4; 
}

return si;
}

void receiveData(site_info* si , int st_choice) {


    printf("Inside receive \n");
    struct sockaddr_in sin2; /* socket struct */
    char *if_name = "wlan0";
    struct ifreq ifr; /* interface struct */
    struct ip_mreq mcast_req;  /* multicast join struct */
    char* mcast_addr = intToIPAddress((si->station_list[st_choice])->multicast_address);
      /* create socket */
    if ((udp_mc_sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("receiver: socket");
        exit(1);
    }

      /* build address data structure */
    memset((char *)&sin2, 0, sizeof(sin2));
    sin2.sin_family = AF_INET;
    sin2.sin_addr.s_addr = htonl(INADDR_ANY);
      //sin2.sin_port = htons(MC_PORT);
    sin2.sin_port = htons((si->station_list[st_choice])->data_port);

    printf("Client connected to : %s:%d\n", mcast_addr , (si->station_list[st_choice])->data_port);

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

// FILE* fptr = fopen("dump.ts" , "wb");

if(fork() == 0) {
    system("sudo ffplay -loglevel quiet pipe");
    exit(0);
}

FILE* fptr = fopen("pipe" , "wb");

if(fptr == NULL) {
    printf("Pipe not opened\n");
    fflush(stdout);
    exit(1);
}
else {
    printf("Pipe opened\n");
    fflush(stdout);
}

int packet = 0;
int len;
int bitRate = (si->station_list[st_choice])->bit_rate;
double reqPack = ((double) (bitRate))  / ((BUF_SIZE * 8.0) / 1000.0);
printf("reqPack = %lf \n", reqPack);
int BUF_PACKETS = (((int)reqPack) + 1) * BUF_TIME;
char* BIG_BUF = malloc(BUF_PACKETS * BUF_SIZE);
printf("Buffered Packets = %d \n", BUF_PACKETS);
fflush(stdout);

int flag = 0;

/*if(setvbuf(fptr, mc_buf, _IOFBF, sizeof mc_buf)) {
        perror("failed to change the buffer of fptr");
        exit(1);
}
*/

while(1) {

  memset(&mcast_saddr, 0, sizeof(mcast_saddr));
  mcast_saddr_len = sizeof(mcast_saddr);
  memset(mc_buf, 0, sizeof(mc_buf));
  if ((len = recvfrom(udp_mc_sock, mc_buf, BUF_SIZE, 0, (struct sockaddr*)&mcast_saddr, 
   &mcast_saddr_len)) < 0) {
    perror("receiver: recvfrom()");
    exit(1);
 }

if(len == 0) {
    printf("End of Transmission\n");
    fclose(fptr);
    fflush(stdout);
    return;
}

 fwrite(mc_buf , 1 , len, fptr);
 fflush(fptr);

/*packet++;

if(packet % BUF_PACKETS == 0)
    fflush(fptr);*/

/*if(packet == BUF_PACKETS){
    fwrite(BIG_BUF , 1 , BUF_SIZE * BUF_PACKETS , fptr);
    fflush(fptr);
    memset(BIG_BUF , 0 , BUF_PACKETS * BUF_SIZE);
    packet = 0;
}


memcpy((BIG_BUF + (BUF_SIZE * packet)) , mc_buf , BUF_SIZE);
packet++;
*/
// if(packet % BUF_PACKETS == 0) fflush(fptr);

/*
if(packet == BUF_PACKETS && !flag) { 
  flag = 1;
  if(fork() == 0) {
    system("ffplay -loglevel quiet dump.ts");
    exit(0);        
}
}*/

}

exit(0);
}

site_info* connectToServer(char* host) {

  struct hostent *hp;
  struct sockaddr_in sin1;
  //char *host;
  char tcp_buf[MAX_LINE];
  int tcp_sock;
  //host = "0.0.0.0\0";
  hp = gethostbyname(host);

  if (!hp) {
    fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
}

bzero((char *)&sin1, sizeof(sin1));
sin1.sin_family = AF_INET;

bcopy(hp->h_addr, (char *)&sin1.sin_addr, hp->h_length);
sin1.sin_port = htons(TCP_PORT);
    /* active open */
if ((tcp_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
}
    /*else
        printf("Client created socket.\n");*/

if (connect(tcp_sock, (struct sockaddr *)&sin1, sizeof(sin1)) < 0)
{
    perror("simplex-talk: connect");
    close(tcp_sock);
    exit(1);
}

memset(tcp_buf , 0 , sizeof(tcp_buf));
recv(tcp_sock, tcp_buf, sizeof(tcp_buf), 0);
close(tcp_sock);

site_info* si = deserialize(tcp_buf);

printf("Site Name : %s\n", si->site_name);
printf("Site Description : %s\n", si->site_desc);
int station_len = si->station_count;
printf("Num of stations = %d \n", station_len);
int i;
for(i=0;i<station_len;i++) {
    printf("Station Number : %d\n", (si->station_list[i])->station_number);
    printf("Station Name : %s\n", (si->station_list[i])->station_name);
    //printf("Multicast Address : %s\n", intToIPAddress((si->station_list[i])->multicast_address));
    //printf("Data Port : %d\n", (si->station_list[i])->data_port);
    //printf("Info Port : %d\n", (si->station_list[i])->info_port);
    //printf("Bit Rate : %d\n", (si->station_list[i])->bit_rate);
    printf("\n");
    fflush(stdout);
}

fflush(stdout);
return si;
}


int main(int argc, char * argv[])   {

    if(argc != 2) {
        printf("Invalid argument usage sudo ./a.out IP_ADDRESS\n");
        exit(1);
    }
  char* server_addr = argv[1];
  site_info* si = connectToServer(server_addr);
  printf("Please choose a station\n");
  int st_choice;
  scanf("%d" , &st_choice);



/********************************UDP PART STARTS HERE*********************************/



  int pid;

  if((pid = fork()) == 0) {
    receiveData(si , st_choice );
    return 0;
}

while(1) {
   printf("P - Pause , R - Resume , C - Station List , X - Terminate\n");
   char choice;
   scanf(" %c",&choice);
        if(choice == 'P' && pid > 0) {// Switch case has some controvorsies 
          kill(pid , SIGTERM);
          system("sudo killall -q ffplay"); // -q quiet -f force
          pid = -1;
      }
      else if(choice == 'R' && pid < 0){
          if((pid = fork()) == 0) {
            receiveData(si , st_choice );
            return 0;
        }
    }
    else if(choice == 'X') {
      printf("Process Terminated\n");
      fflush(stdout);
          // kill(pid , SIGTERM);
      system("sudo killall -q ffplay");                    
      return 0;
  }
  else if(choice == 'C') {
      if(pid > 0) {
        kill(pid , SIGTERM);
    }
    system("sudo killall -q ffplay");
    si = connectToServer(server_addr);
    printf("Please choose a station\n");
    fflush(stdout);
    scanf("%d" , &st_choice);
    if((pid = fork()) == 0) {
        receiveData(si , st_choice);
        return 0;
    }
}
else {
  printf("Invalid Operation You have entered = %c current pid = %d \n" , choice , pid);
  fflush(stdout);
}
}
return 0;  
}