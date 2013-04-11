#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <janssson.h>

const int KB_SIZE = 2048;

int main() {
  int server_socket_fd;
  int bytes_sent;
  int bytes_recieved;
  int loop_var;
  struct sockaddr_in external_address;
  char recieve_buffer[KB_SIZE * 1024];
  char set_size[40] = "SETCWND 3";
  char send_data[40] = "SENDDAT";

  for(loop_var = 0; loop_var < 10; loop_var++){

    if((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
      perror("socket");
      exit(1);
    }

    external_address.sin_family = AF_INET;    // host byte order 
    external_address.sin_port = htons(5010);  // short, network byte order 
    external_address.sin_addr.s_addr = inet_addr("54.225.93.141");
    memset(&(external_address.sin_zero), '\0', 8);  // zero the rest of the struct 

    fprintf(stderr, "Connecting\n");
    if (connect(server_socket_fd, (struct sockaddr *)&external_address, sizeof(struct sockaddr)) == -1) {
      perror("connect");
      exit(1);
    }

    fprintf(stderr, "About to send data\n");
    if((bytes_sent = send(server_socket_fd, set_size, 11, 0)) == -1){
      perror("send");
      exit(1);
    }
    fprintf(stderr, "Data sent\n");
    close(server_socket_fd);

    if((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
      perror("socket");
      exit(1);
    }

    if (connect(server_socket_fd, (struct sockaddr *)&external_address, sizeof(struct sockaddr)) == -1) {
      perror("connect");
      exit(1);
    }

    fprintf(stderr, "About to send data\n");
    if((bytes_sent = send(server_socket_fd, send_data, 8, 0)) == -1){
      perror("send");
      exit(1);
    }
    fprintf(stderr, "Data sent\n");

    if ((bytes_recieved=recv(server_socket_fd, recieve_buffer, KB_SIZE * 1024, MSG_WAITALL)) == -1) {
      perror("recv");
      exit(1);
    }

    printf("I got %i bytes\n\n", bytes_recieved);
    close(server_socket_fd);
  }


  return 0;
}

