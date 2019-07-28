#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#define MAXBUFFER 1024
int max(int x, int y) {
    if (x > y)
        return x;
    else
        return y;
}
char users[64][16];
int usersds[64]={0};
int numusers=0;
static int myCompare(const void* a, const void* b)
{

    // setting up rules for comparison
    return strcmp(*(const char**)a, *(const char**)b);
}

// Function to sort the array
void sort(const char* arr[], int n)
{
    // calling qsort function to sort the array
    // with the help of Comparator
    qsort(arr, n, sizeof(const char*), myCompare);
}

void *comm(void *argv){
    int *sd=(int *)argv;
    int n;
    char buffer[MAXBUFFER];
    char id[16]="";
    int correctsharesd=-1;
    int sharelength=0;
    while(1){
        fflush(stdout);
        n= recv(*sd,buffer,MAXBUFFER,0);
        if(n<=0){
            printf("CHILD %u: Client disconnected\n", *sd);
            fflush(stdout);
            return 0;
		}
        else{


            if(strncmp(buffer, "LOGIN",5)==0){
                printf("CHILD %u: Rcvd ",*sd);
                printf("LOGIN request for userid ");
                int avail=0;
                for(int i=0;i<64;i++){
                    if(usersds[i]==*sd){
                        avail=1;

                    }
                }
                if(avail==1){
                    printf("CHILD %u: ERROR socket in use\n",*sd);
                }
                else{



                    int i=6;

                    while(buffer[i]!='\n'){
                        id[i-6]=buffer[i];
                        i++;
                    }

                    id[i-6]='\0';
                    printf("%s\n",id);
                    if(strlen(id)<4){
                        send(*sd,"ERROR Invalid userid\n",strlen("ERROR Invalid userid\n"),0);
                        printf("CHILD %u: Sent ERROR (Invalid userid)\n",*sd);
                    }
                    else{
                        int already=0;
                        for(int q=0;q<64;q++){
                            if(strcmp(id,users[q])==0){
                                already=1;

                            }
                        }
                        if(already==1){
                            send(*sd,"ERROR Already connected\n",strlen("ERROR Already connected\n"),0);
                            printf("CHILD %u: Sent ERROR (Already connected)\n",*sd);
                        }
                        else{




                            for(int j=0;j<64;j++){
                                if(strcmp(users[j],"")==0){
                                    strcpy(users[j],id);
                                    usersds[j]=*sd;
                                    break;
                                }
                            }

                            numusers++;

                            send(*sd,"OK!\n",strlen("OK!\n"),0);
                        }
                    }
                }

            }

            else if(strncmp(buffer,"WHO",3)==0){
                printf("CHILD %u: Rcvd ",*sd);
                printf("WHO request\n");
                const char* copy[64];
                for(int i=0;i<64;i++){
                  copy[i]=users[i];

                }
                int size=sizeof(copy)/sizeof(copy[0]);
                sort(copy,size);
                char msg[100]="OK!\n";
                for(int j=0;j<64;j++){
                    if(strcmp(copy[j],"")!=0){

                        strcat(msg,copy[j]);
                        strcat(msg,"\n");
                    }

                }
                send(*sd,msg,strlen(msg),0);
            }
            else if(strncmp(buffer,"LOGOUT",6)==0){
                printf("CHILD %u: Rcvd ",*sd);
                printf("LOGOUT request\n");
                send(*sd,"OK!\n",strlen("OK!\n"),0);

                for(int j=0;j<64;j++){
                    if(strcmp(users[j],id)==0){
                        strcpy(users[j],"");
                        usersds[j]=0;
                    }
                }


                numusers--;


            }
            else if(strncmp(buffer,"SEND",4)==0){
                printf("CHILD %u: Rcvd ",*sd);

                printf("SEND request to userid ");
                char message[1000]="";
                int i=5;
                char recid[16]="";
                while(buffer[i]!=' '){
                    recid[i-5]=buffer[i];
                    i++;
                }
                recid[i-5]='\0';
         
                int correctsd=-1;
                for(int q=0;q<64;q++){
                    if(strcmp(recid,users[q])==0){
                        correctsd=usersds[q];

                        break;
                    }
                }
                    char leng[4]="";

                    int g=0;
                    while(buffer[i]!='\n'){

                        leng[g]=buffer[i];

                        i++;
                        g++;
                    }
                    message[0]=' ';
                    i++;
                    leng[g]='\0';
                    int length=atoi(leng);



                    g=1;
                    while(g<=length){
                        message[g]=buffer[i];
                        i++;
                        g++;
                    }
                    message[g]='\0';


                    //build string with variables
                    char* tosend = malloc(1500*sizeof(char));

                    strcat(tosend,"FROM ");
                    strcat(tosend,id);

                    strcat(tosend,leng);
                    strcat(tosend,message);


                    strcat(tosend,"\n");

                    if(correctsd==-1){
                        send(*sd,"ERROR Unknown userid\n",strlen("ERROR Unknown userid\n"),0);
                        printf("CHILD %u: Sent ERROR (Unknown userid)\n",*sd);
                    }
                    else if(length>990 || length<1){
                      send(*sd,"ERROR Invalid msglen\n",strlen("ERROR Invalid msglen\n"),0);
                      printf("CHILD %u: Sent ERROR (Invalid msglen)\n",*sd);
                    }
                    else{
                      send(*sd,"OK!\n",strlen("OK!\n"),0);
                      send(correctsd,tosend,strlen(tosend),0);
                    }


            }
              else if(strncmp(buffer,"BROADCAST",9)==0){
                  printf("CHILD %u: Rcvd ",*sd);
                  printf("BROADCAST request\n");
                  char message[1000]="";
                  char leng[4]="";
                  int g=0;
                  int i=9;

                  while(buffer[i]!='\n'){

                      leng[g]=buffer[i];

                      i++;
                      g++;
                  }
                  leng[g]='\0';
                  int length=atoi(leng);



                  g=0;

                  while(g<=length){
                      message[g]=buffer[i];

                      i++;
                      g++;
                  }
                  message[0]=' ';
                  message[g]='\0';
                  char* tosend = malloc(1500*sizeof(char));

                  strcat(tosend,"FROM ");
                  strcat(tosend,id);

                  strcat(tosend,leng);
                  strcat(tosend,message);


                  strcat(tosend,"\n");
                  send(*sd,"OK!\n",strlen("OK!\n"),0);
                  for(i=0;i<64;i++){
                    if(usersds[i]!=0){

                      send(usersds[i],tosend,strlen(tosend),0);
                    }
                  }
                }
                else if(strncmp(buffer,"SHARE",5)==0){
                  printf("CHILD %u: Rcvd ",*sd);
                  printf("SHARE request\n");

                  char recipient[16]="";
                  char filelen[5000]="";
                  int i=6;
                  while(buffer[i]!=' '){
                      recipient[i-6]=buffer[i];

                      i++;
                  }
                  int g=0;
                  recipient[i]='\0';
                  while(buffer[i]!='\n'){

                      filelen[g]=buffer[i];

                      i++;
                      g++;
                  }


                  filelen[g]='\0';
                  sharelength=atoi(filelen);

                  for(int q=0;q<64;q++){
                      if(strcmp(recipient,users[q])==0){

                          correctsharesd=usersds[q];

                          break;
                      }
                  }
                  send(*sd,"OK!\n",strlen("OK!\n"),0);
                  char confirm[1000]="";
                  strcat(confirm,"SHARE ");
                  strcat(confirm,id);
                  strcat(confirm,filelen);
                  strcat(confirm,"\n");

                  send(correctsharesd,confirm,strlen(confirm),0);


                }
                else{
                  int times=sharelength/1024;
                  send(*sd,"OK!\n",strlen("OK!\n"),0);
                  while(times>=0){

                    if(sharelength<=1024){
                      send(correctsharesd,buffer,sharelength,0);
                    }
                    else{
                      send(correctsharesd,buffer,1024,0);
                    }

                    times--;
                    sharelength-=1024;
                  }

                }
        }

    }
}

int main(int argc, char *argv[]){
    setvbuf( stdout, NULL, _IONBF, 0 );
    if(argc!=2){
        fprintf(stderr, "MAIN: ERROR Invalid argument(s)\n");
		fprintf(stderr, "MAIN: USAGE a.out <port>\n");
		return EXIT_FAILURE;
	}
    fd_set readfds;
    unsigned short port = atoi(argv[1]);
    int tcpsd=socket(AF_INET,SOCK_STREAM, 0);
    int udpsd=socket(AF_INET,SOCK_DGRAM, 0);
    if(tcpsd<0){
        fprintf(stderr,"MAIN: ERROR tcp socket failed\n");
        return EXIT_FAILURE;
    }
    if(udpsd<0){
        fprintf(stderr,"MAIN: ERROR udp socket failed\n");
        return EXIT_FAILURE;
    }
    printf("MAIN: Started server\n");

    //create server
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);
    if(bind(tcpsd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        fprintf(stderr,"MAIN: ERROR tcp bind fail\n");
        return EXIT_FAILURE;
    }
    listen(tcpsd,10);

    if(bind(udpsd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        fprintf(stderr,"MAIN: ERROR udp bind fail\n");
        return EXIT_FAILURE;
    }
    FD_ZERO( &readfds );

    printf("MAIN: Listening for TCP connections on port: %d\n", port);

	printf("MAIN: Listening for UDP datagrams on port: %d\n", port);
    struct sockaddr_in cli;


    int n=0;
    char buffer[MAXBUFFER];
    int maxfdp=max(tcpsd,udpsd)+1;

    pthread_t *tid=(pthread_t *) malloc(10*sizeof(pthread_t));
    while(1){

        FD_SET(tcpsd,&readfds);
        FD_SET(udpsd,&readfds);

        int ready=select(maxfdp,&readfds,NULL,NULL,NULL);
        if(ready>0){
            if(FD_ISSET(tcpsd,&readfds)){
                int *newsocket = (int *) malloc(sizeof(int));
                int len = sizeof(cli);
                *newsocket = accept(tcpsd, (struct sockaddr *)&cli, (socklen_t*)&len);
                printf("MAIN: Rcvd incoming TCP connection from: %s\n", inet_ntoa((struct in_addr)cli.sin_addr));
                printf("%d",*newsocket);
                int rc=pthread_create(&tid[n],NULL,&comm,(void *)newsocket);
                n++;
                if(rc!=0){
                    fprintf(stderr,"MAIN: ERROR Failed to create thread\n");
                    return EXIT_FAILURE;
                }
            }
            if(FD_ISSET(udpsd, &readfds)){
                int len = sizeof(cli);
                n = recvfrom(udpsd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli, (socklen_t *)&len);
                if(n < 0){
                    fprintf(stderr, "MAIN: ERROR Failed to create recvfrom\n");
                }
                else{
                    printf("MAIN: Rcvd incoming UDP datagram from: %s\n", inet_ntoa((struct in_addr)cli.sin_addr));
                    if(strncmp(buffer,"WHO",3)==0){
                        printf("MAIN: Rcvd WHO request\n");

                        char msg[MAXBUFFER]="OK!\n";
                        for(int j=0;j<64;j++){
                            if(strcmp(users[j],"")!=0){
                                strcat(msg,users[j]);
                                strcat(msg,"\n");
                            }

                        }
                        sendto(udpsd,msg,strlen(msg),0,(struct sockaddr *)&cli,len);
                    }
                    else if(strncmp(buffer,"SEND",4)==0){
                      printf("MAIN: Rcvd SEND request\n");
                     sendto(udpsd,"ERROR SEND not supported over UDP\n",strlen("ERROR SEND not supported over UDP\n"),0,(struct sockaddr *)&cli,len);

                    }
                    else if(strncmp(buffer,"BROADCAST",9)==0){
                        printf("MAIN: Rcvd BROADCAST request\n");
                        char message[1000]="";
                        char leng[4]="";
                        int g=0;
                        int i=9;

                        while(buffer[i]!='\n'){

                            leng[g]=buffer[i];

                            i++;
                            g++;
                        }
                        leng[g]='\0';
                        int length=atoi(leng);



                        g=0;

                        while(g<=length){
                            message[g]=buffer[i];

                            i++;
                            g++;
                        }
                        message[0]=' ';
                        message[g]='\0';
                        char* tosend = malloc(1500*sizeof(char));

                        strcat(tosend,"FROM UDP-client");


                        strcat(tosend,leng);
                        strcat(tosend,message);

                        strcat(tosend,"\n");

                        sendto(udpsd,"OK!\n",strlen("OK!\n"),0,(struct sockaddr *)&cli,len);
                        for(i=0;i<64;i++){
                          if(usersds[i]!=0){

                            send(usersds[i],tosend,strlen(tosend),0);
                          }
                        }
                      }
                      else if(strncmp(buffer,"SHARE",5)==0){
                          printf("MAIN: Rcvd SHARE request\n");
                         sendto(udpsd,"ERROR SHARE not supported over UDP\n",strlen("ERROR SHARE not supported over UDP\n"),0,(struct sockaddr *)&cli,len);
                       }
                }
            }
        }

    }

}
