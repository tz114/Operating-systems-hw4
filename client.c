/*
 * OpSys HW4 Submitty client clone, from @618 on Piazza
 *
 * Original code (including tests 4, 5, and 6) by Anonymous.
 * Tests 7 and 8 added by Pragati Pant.
 * Test 9 added by Eric Johnson.
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void send_tcp(int sd, char *msg) {

  printf("sending [%s]\n", msg);
  //fprintf(stderr, "DEBUG: message is %lu bytes long\n", strlen(msg));
  int n = write(sd, msg, strlen(msg)); /* or send()/recv() */

  if (n < strlen(msg)) {
    perror("write() failed");
    return; // EXIT_FAILURE;
  }
}

void broadcast_tcp(int sd, char *msg) {

  printf("sending [%s]\n", msg);
  //fprintf(stderr, "DEBUG: message is %lu bytes long\n", strlen(msg));
  int n = write(sd, msg, strlen(msg)); /* or send()/recv() */

  if (n < strlen(msg)) {
    perror("write() failed");
    return; // EXIT_FAILURE;
  }
}

void recv_tcp(int sd, char *recv) {
  int recv_len = strlen(recv);
  char buffer[recv_len + 1];
  //fprintf(stderr, "DEBUG: waiting for %d bytes [%s]\n", recv_len, recv);
  int n = read(sd, buffer, recv_len); /* BLOCKING */

  //fprintf(stderr, "DEBUG: in the buffer: %.*s\n", n, buffer);
  if (n == -1) {
    perror("read() failed");
    return; // EXIT_FAILURE;
  } else if (n == 0) {
    printf("Rcvd no data; also, server socket was closed\n");
  } else /* n > 0 */
  {
    // buffer[n] = '\0'; /* assume we rcvd text-based data */
    printf("TCP rcvd [%.*s]\n", n, buffer);
  }
}

void recv_data_tcp(int sd, int recv_len) {
  char buffer[recv_len];
//  fprintf(stderr, "DEBUG: waiting for %d bytes\n", recv_len);
  int n = read(sd, buffer, recv_len); /* BLOCKING */

  if (n == -1) {
    perror("read() failed");
    return; // EXIT_FAILURE;
  } else if (n == 0) {
    printf("Rcvd no data; also, server socket was closed\n");
  } else /* n > 0 */
  {
    printf("TCP rcvd %d bytes\n", n);
  }
}

int connect_tcp(unsigned short port) {
  /* create TCP client socket (endpoint) */
  int sd = socket(PF_INET, SOCK_STREAM, 0);

  if (sd == -1) {
    perror("socket() failed");
    exit(EXIT_FAILURE);
  }

#if 1
  struct hostent *hp = gethostbyname("localhost"); /* 127.0.0.1 */
  // struct hostent * hp = gethostbyname( "127.0.0.1" );
  // struct hostent * hp = gethostbyname( "128.113.126.29" );
#endif

  // struct hostent * hp = gethostbyname( "linux04.cs.rpi.edu" );

  if (hp == NULL) {
    fprintf(stderr, "ERROR: gethostbyname() failed\n");
    return EXIT_FAILURE;
  }

  struct sockaddr_in server;
  server.sin_family = PF_INET;
  memcpy((void *)&server.sin_addr, (void *)hp->h_addr, hp->h_length);
  // unsigned short port = atoi(argv[1]);
  server.sin_port = htons(port);

  printf("Server address is %s\n", inet_ntoa(server.sin_addr));

  // printf( "connecting to server.....\n" );
  if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
    perror("connect() failed");
    return EXIT_FAILURE;
  }
  printf("Connected to the server via TCP\n");
  return sd;
}

struct udp_connection {
  int sd;
  struct sockaddr_in servaddr;
};

void send_udp(struct udp_connection *udp, char *msg) {
  printf("sending [%s]\n", msg);
  sendto(udp->sd, msg, strlen(msg), 0, (struct sockaddr *)&udp->servaddr,
         sizeof(udp->servaddr));
}

void recv_udp(struct udp_connection *udp, char *recv) {
  int recv_len = strlen(recv);
  char buffer[recv_len + 1];
  int len = sizeof(udp->servaddr);
  int n = recvfrom(udp->sd, (char *)buffer, recv_len, 0,
                   (struct sockaddr *)&udp->servaddr, (socklen_t *)&len);
  if (n == -1) {
    perror("recvfrom() failed");
    return; // EXIT_FAILURE;
  } else {  /* n >= 0 */
    // buffer[recv_len] = '\0';
    printf("UDP rcvd [%.*s]\n", n, buffer);
  }
}

// https://www.geeksforgeeks.org/udp-server-client-implementation-c/
struct udp_connection connect_udp(unsigned short port) {
  int sd;
  // char buffer[MAXLINE];
  // char *hello = "Hello from client";
  struct sockaddr_in servaddr;

  // Creating socket file descriptor
  if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = PF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  struct udp_connection udp;
  udp.sd = sd;
  udp.servaddr = servaddr;

  return udp;
}

void test4(unsigned short port) {

  int tcp1 = connect_tcp(port);
  send_tcp(tcp1, "LOGIN Morty\n");
  recv_tcp(tcp1, "OK!\n");
  send_tcp(tcp1, "WHO\n");
  recv_tcp(tcp1, "OK!\nMorty\n");

  int tcp2 = connect_tcp(port);
  send_tcp(tcp2, "LOGIN Rick\n");
  recv_tcp(tcp2, "OK!\n");
  send_tcp(tcp2, "WHO\n");
  recv_tcp(tcp2, "OK!\nMorty\nRick\n");

  send_tcp(tcp1, "LOGOUT\n");
  recv_tcp(tcp1, "OK!\n");
  send_tcp(tcp1, "LOGIN Beth\n");
  recv_tcp(tcp1, "OK!\n");

  struct udp_connection udp_c = connect_udp(port);
  struct udp_connection *udp = &udp_c;
  send_udp(udp, "WHO\n");
  recv_udp(udp, "OK!\nBeth\nRick\n");

  send_tcp(tcp1, "LOGOUT\n");
  recv_tcp(tcp1, "OK!\n");
  close(tcp1);

  send_tcp(tcp2, "LOGOUT\n");
  recv_tcp(tcp2, "OK!\n");
  close(tcp2);

  send_udp(udp, "WHO\n");
  recv_udp(udp, "OK!\n");

  close(udp->sd);
  return;
}

void test5(unsigned short port) {

  int tcp1 = connect_tcp(port);
  send_tcp(tcp1, "LOGIN Rick\n");
  recv_tcp(tcp1, "OK!\n");

  int tcp2 = connect_tcp(port);
  send_tcp(tcp2, "LOGIN Rick\n");
  recv_tcp(tcp2, "ERROR Already connected\n");
  send_tcp(tcp2, "LOGIN Summer\n");
  recv_tcp(tcp2, "OK!\n");

  send_tcp(tcp1, "WHO\n");
  recv_tcp(tcp1, "OK!\nRick\nSummer\n");

  close(tcp1);
  close(tcp2);
}

void test6(unsigned short port) {

  int tcp1 = connect_tcp(port);
  send_tcp(tcp1, "LOGIN Rick\n");
  recv_tcp(tcp1, "OK!\n");
  send_tcp(tcp1, "SEND Rick 22\nI'm talking to myself!\n");
  recv_tcp(tcp1, "OK!\n");
  recv_tcp(tcp1, "FROM Rick 22 I'm talking to myself!\n");

  int tcp2 = connect_tcp(port);
  send_tcp(tcp2, "LOGIN G\n");
  recv_tcp(tcp2, "ERROR Invalid userid\n");
  close(tcp2);

  int tcp3 = connect_tcp(port);
  send_tcp(tcp3, "LOGIN Summer\n");
  recv_tcp(tcp3, "OK!\n");
  send_tcp(tcp3, "SEND Rick 25\nStop talking to yourself!\n");
  recv_tcp(tcp3, "OK!\n");

  recv_tcp(tcp1, "FROM Summer 25 Stop talking to yourself!\n");
  send_tcp(tcp1, "LOGOUT\n");
  recv_tcp(tcp1, "OK!\n");
  close(tcp1);

  send_tcp(tcp3, "SEND Rick 20\nAre you still there?\n");
  recv_tcp(tcp3, "ERROR Unknown userid\n");
  send_tcp(tcp3, "SEND Summer 26\nNow I'm talking to myself!\n");
  recv_tcp(tcp3, "OK!\n");
  recv_tcp(tcp3, "FROM Summer 26 Now I'm talking to myself!\n");

  struct udp_connection udp_c = connect_udp(port);
  struct udp_connection *udp = &udp_c;
  send_udp(udp, "WHO\n");
  recv_udp(udp, "OK!\nSummer\n");

  close(tcp3);
}
/*[TEST CASE #7] 12 POINTS
TCP Client #1: LOGIN Rick\n
               SEND Morty 21\nAre you there, Morty?
TCP Client #2: LOGIN Morty\n
               WHO\n
TCP Client #1: SEND Morty 24\nAha, there u are, Morty!
TCP Client #2: SEND Rick 27\nYes, idiot, I'm right here!
               BROADCAST 21\nAaaaaaaaaaaaaaaaaagh!
TCP Client #1: LOGOUT\n
TCP Client #2: WHO\n
               LOGOUT\n*/
void test7(unsigned short port) {
  int tcp1 = connect_tcp(port);
  send_tcp(tcp1, "LOGIN Rick\n");
  recv_tcp(tcp1, "OK!\n");
  send_tcp(tcp1, "SEND Morty 21\nAre you there, Morty?\n");
  recv_tcp(tcp1, "ERROR Unknown userid\n");

  int tcp2 = connect_tcp(port);
  send_tcp(tcp2, "LOGIN Morty\n");
  recv_tcp(tcp2, "OK!\n");
  send_tcp(tcp2, "WHO\n");
  recv_tcp(tcp2, "OK!\nMorty\nRick\n");

  send_tcp(tcp1, "SEND Morty 24\nAha, there u are, Morty!\n");
  recv_tcp(tcp1, "OK!\n");

  recv_tcp(tcp2, "FROM Rick 24 Aha, there u are, Morty!\n");

  send_tcp(tcp2, "SEND Rick 27\nYes, idiot, I'm right here!\n");
  recv_tcp(tcp2, "OK!\n");

  recv_tcp(tcp1, "FROM Morty 27 Yes, idiot, I'm right here!\n");

  broadcast_tcp(tcp2, "BROADCAST 21\nAaaaaaaaaaaaaaaaaagh!\n");
  recv_tcp(tcp2, "OK!\n");
  recv_tcp(tcp1, "FROM Morty 21 Aaaaaaaaaaaaaaaaaagh!\n");
  recv_tcp(tcp2, "FROM Morty 21 Aaaaaaaaaaaaaaaaaagh!\n");

  send_tcp(tcp1, "LOGOUT\n");
  recv_tcp(tcp1, "OK!\n");
  close(tcp1);

  send_tcp(tcp2, "WHO\n");
  recv_tcp(tcp2, "OK!\nMorty\n");
  send_tcp(tcp2, "LOGOUT\n");
  recv_tcp(tcp2, "OK!\n");
  close(tcp2);
}
/*[TEST CASE #8] 12 POINTS
TCP Client #1: LOGIN Rick\n
TCP Client #2: LOGIN Morty\n
               WHO\n
TCP Client #1: SEND Morty 21\nHere's a short fable!
               SHARE Morty 917\n
               <bytes-from-mouse.txt>
TCP Client #2: SEND Rick 10\meaningful
TCP Client #1: LOGOUT\n
TCP Client #2: LOGOUT\n*/

void test8(unsigned short port) {
  int tcp1 = connect_tcp(port);
  send_tcp(tcp1, "LOGIN Rick\n");
  recv_tcp(tcp1, "OK!\n");

  int tcp2 = connect_tcp(port);
  send_tcp(tcp2, "LOGIN Morty\n");
  recv_tcp(tcp2, "OK!\n");
  send_tcp(tcp2, "WHO\n");
  recv_tcp(tcp2, "OK!\nMorty\nRick\n");

  send_tcp(tcp1, "SEND Morty 21\nHere's a short fable!\n");
  recv_tcp(tcp1, "OK!\n");
  recv_tcp(tcp2, "FROM Rick 21 Here's a short fable!\n");

  send_tcp(tcp1, "SHARE Morty 917\n");
  recv_tcp(tcp1, "OK!\n");
  recv_tcp(tcp2, "SHARE Rick 917\n");

  printf("transmitting mouse.txt...\n");
  send_tcp(
      tcp1,
      "Once when a Lion was asleep a little Mouse began running up and down "
      "upon him; this soon wakened the Lion, who placed his huge paw upon "
      "him, "
      "and opened his big jaws to swallow him. \"Pardon, O King,\" cried the "
      "little Mouse: \"forgive me this time, I shall never forget it: who "
      "knows but what I may be able to do you a turn some of these days?\" "
      "The "
      "Lion was so tickled at the idea of the Mouse being able to help him, "
      "that he lifted up his paw and let him go. Some time after the Lion "
      "was "
      "caught in a trap, and the hunters, who desired to carry him alive to "
      "the King, tied him to a tree while they went in search of a wagon to "
      "carry him on. Just then the little Mouse happened to pass by, and "
      "seeing the sad plight in which the Lion was, sent up to him and soon "
      "gnawed away the ropes that bound the King of the Beasts. \"Was I not "
      "right?\" said the little Mouse.\n                \"LITTLE FRIENDS MAY "
      "PROVE GREAT FRIENDS.\"\n\n");

  // bytes are sent
  recv_tcp(tcp1, "OK!\n");

  recv_tcp(
      tcp2,
      "Once when a Lion was asleep a little Mouse began running up and down "
      "upon him; this soon wakened the Lion, who placed his huge paw upon "
      "him, "
      "and opened his big jaws to swallow him. \"Pardon, O King,\" cried the "
      "little Mouse: \"forgive me this time, I shall never forget it: who "
      "knows but what I may be able to do you a turn some of these days?\" "
      "The "
      "Lion was so tickled at the idea of the Mouse being able to help him, "
      "that he lifted up his paw and let him go. Some time after the Lion "
      "was "
      "caught in a trap, and the hunters, who desired to carry him alive to "
      "the King, tied him to a tree while they went in search of a wagon to "
      "carry him on. Just then the little Mouse happened to pass by, and "
      "seeing the sad plight in which the Lion was, sent up to him and soon "
      "gnawed away the ropes that bound the King of the Beasts. \"Was I not "
      "right?\" said the little Mouse.\n                \"LITTLE FRIENDS MAY "
      "PROVE GREAT FRIENDS.\"\n\n");

  send_tcp(tcp2, "SEND Rick 10\nmeaningful\n");
  recv_tcp(tcp2, "OK!\n");

  recv_tcp(tcp1, "FROM Morty 10 meaningful\n");

  send_tcp(tcp1, "LOGOUT\n");
  recv_tcp(tcp1, "OK!\n");
  close(tcp1);

  send_tcp(tcp2, "LOGOUT\n");
  recv_tcp(tcp2, "OK!\n");
  close(tcp2);
}

/*[TEST CASE #9] 12 POINTS
TCP Client #1: LOGIN Rick\n
TCP Client #2: LOGIN Morty\n
   UDP Client: BROADCAST 32\nACK! I'M CUCKOO FOR COCOA PUFFS!
TCP Client #1: SEND Morty 23\nCheck out this picture!
               SHARE Morty 100774\n
               <bytes-from-sonny1978.jpg>
TCP Client #2: SEND Rick 11\nI'm hungry.
               LOGOUT\n
   UDP Client: WHO\n*/
void test9(unsigned short port) {
  int tcp1 = connect_tcp(port);
  send_tcp(tcp1, "LOGIN Rick\n");
  recv_tcp(tcp1, "OK!\n");

  int tcp2 = connect_tcp(port);
  send_tcp(tcp2, "LOGIN Morty\n");
  recv_tcp(tcp2, "OK!\n");

  struct udp_connection udp_c = connect_udp(port);
  struct udp_connection *udp = &udp_c;
  send_udp(udp, "BROADCAST 32\nACK! I'M CUCKOO FOR COCOA PUFFS!\n");
  recv_udp(udp, "OK!\n");
  recv_tcp(tcp1, "FROM UDP-client 32 ACK! I'M CUCKOO FOR COCOA PUFFS!\n");
  recv_tcp(tcp2, "FROM UDP-client 32 ACK! I'M CUCKOO FOR COCOA PUFFS!\n");

  send_tcp(tcp1, "SEND Morty 23\nCheck out this picture!\n");
  recv_tcp(tcp1, "OK!\n");

  recv_tcp(tcp2, "FROM Rick 23 Check out this picture!\n");

  send_tcp(tcp1, "SHARE Morty 100774\n");
  recv_tcp(tcp1, "OK!\n");
  recv_tcp(tcp2, "SHARE Rick 100774\n");
  printf("transmitting sonny1978.jpg...\n");

  // read file from current directory
  int fd = open("sonny1978.jpg", O_RDONLY);
  if (fd == -1) {
    perror("opening sonny1978.jpg failed");
    return;
  }
  int filelen = 100774;
  char chunk[1024];
  // send chunks of the file in a loop
  while (filelen > 0) {
    int chunk_len = filelen > 1024 ? 1024 : filelen;
    if (read(fd, chunk, chunk_len) < chunk_len) {
      perror("read() failed");
      close(fd);
      return;
    }
    int n = write(tcp1, chunk, chunk_len);
    if (n != chunk_len) {
      perror("write() failed");
      close(fd);
      return;
    }
    filelen -= chunk_len;
    recv_tcp(tcp1, "OK!\n");
    // Submitty doesn't check if the received data is right, so we won't either
    recv_data_tcp(tcp2, chunk_len);
  }
  close(fd);

  send_tcp(tcp2, "SEND Rick 11\nI'm hungry.\n");
  recv_tcp(tcp2, "OK!\n");
  recv_tcp(tcp1, "FROM Morty 11 I'm hungry.\n");

  send_tcp(tcp2, "LOGOUT\n");
  recv_tcp(tcp2, "OK!\n");
  close(tcp2);

  send_udp(udp, "WHO\n");
  recv_udp(udp, "OK!\nRick\n");
  close(tcp1);
}

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    printf("Usage: %s <server port> [test number]\n", argv[0]);
    return EXIT_FAILURE;
  }
  unsigned short port = atoi(argv[1]);
  int test_num = 0;
  if (argc == 3) {
    test_num = atoi(argv[2]);
  }
  if (test_num == 4 || test_num == 0) {
    printf("TEST #4\n");
    test4(port);
  }
  if (test_num == 5 || test_num == 0) {
    printf("TEST #5\n");
    test5(port);
  }
  if (test_num == 6 || test_num == 0) {
    printf("TEST #6\n");
    test6(port);
  }
  if (test_num == 7 || test_num == 0) {
    printf("TEST #7\n");
    test7(port);
  }
  if (test_num == 8 || test_num == 0) {
    printf("TEST #8\n");
    test8(port);
  }
  if (test_num == 9 || test_num == 0) {
    printf("TEST #9\n");
    test9(port);
  }

  return EXIT_SUCCESS;
}
