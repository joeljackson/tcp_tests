#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <jansson.h>

#include "client.h"
#include "string.h"
#include "server.h"


int main() { 
  json_t *configuration;
  const char *host;
  configuration = read_configuration();

  host = json_string_value(json_object_get(configuration, "host"));
  fprintf(stderr, "host: %s\n", host);

  return 0;
}

void read_from_server(int iterations, int size){
  int server_socket_fd;
  int bytes_sent;
  int bytes_recieved;
  int loop_var;
  char *recieve_buffer;
  struct sockaddr_in external_address;

  recieve_buffer = malloc(size * 1024);

  external_address.sin_family = AF_INET;
  external_address.sin_port = htons(5010);
  external_address.sin_addr.s_addr = inet_addr("54.225.93.141");
  memset(&(external_address.sin_zero), '\0', 8);

  for(loop_var = 0; loop_var < iterations; loop_var++){
    if((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
      perror("socket");
      exit(1);
    }

    if (connect(server_socket_fd, (struct sockaddr *)&external_address, sizeof(struct sockaddr)) == -1) {
      perror("connect");
      exit(1);
    }

    fprintf(stderr, "About to send data\n");
    if((bytes_sent = send(server_socket_fd, SEND_DATA, 8, 0)) == -1){
      perror("send");
      exit(1);
    }
    fprintf(stderr, "Data sent\n");

    if ((bytes_recieved=recv(server_socket_fd, recieve_buffer, size * 1024, MSG_WAITALL)) == -1) {
      perror("recv");
      exit(1);
    }

    printf("I got %i bytes\n\n", bytes_recieved);
    close(server_socket_fd);
  }
}

void set_congestion_window(int packets){
  int server_socket_fd;
  int bytes_sent;
  char command[20];
  struct sockaddr_in external_address;

  if((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
    perror("socket");
    exit(1);
  }

  external_address.sin_family = AF_INET;
  external_address.sin_port = htons(5010);
  external_address.sin_addr.s_addr = inet_addr("54.225.93.141");
  memset(&(external_address.sin_zero), '\0', 8);

  snprintf(command, 20, "%s %d", SET_CONGESTION_WINDOW, packets);

  if (connect(server_socket_fd, (struct sockaddr *)&external_address, sizeof(struct sockaddr)) == -1) {
    perror("connect");
    exit(1);
  }

  if((bytes_sent = send(server_socket_fd, command, 11, 0)) == -1){
    perror("send");
    exit(1);
  }

  close(server_socket_fd);
}

void set_send_size(int size){
  int server_socket_fd;
  int bytes_sent;
  char command[20];
  struct sockaddr_in external_address;

  if((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
    perror("socket");
    exit(1);
  }

  external_address.sin_family = AF_INET;
  external_address.sin_port = htons(5010);
  external_address.sin_addr.s_addr = inet_addr("54.225.93.141");
  memset(&(external_address.sin_zero), '\0', 8);

  snprintf(command, 20, "%s %d", SET_DATA_SIZE, size);

  if (connect(server_socket_fd, (struct sockaddr *)&external_address, sizeof(struct sockaddr)) == -1) {
    perror("connect");
    exit(1);
  }

  if((bytes_sent = send(server_socket_fd, command, 11, 0)) == -1){
    perror("send");
    exit(1);
  }

  close(server_socket_fd);
}

json_t* read_configuration(){
  int configuration_length;
  int chars_read;
  char line[500];
  json_t *root;
  json_error_t error;
  String *configuration;
  FILE *configuration_file;

  configuration = new_string();
  configuration_file = fopen("./configuration.json", "r");

  while(fgets(line, 500, configuration_file) != NULL){
    cat_string(configuration, line, 500);
  }
  fclose(configuration_file);  /* close the file prior to exiting the routine */

  root = json_loads(configuration->buffer, 0, &error);
  free_string(configuration);
  return root;
}




