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
  const json_t *tests;
  configuration = read_configuration();

  if(!json_is_string(json_object_get(configuration, "host"))){
    fprintf(stderr, "Must have string host\n");
    exit(1);
  }
  host = json_string_value(json_object_get(configuration, "host"));

  tests = json_object_get(configuration, "tests");
  if(!json_is_array(tests)){
    fprintf(stderr, "Must have array of tests\n");
    exit(1);
  }

  execute_tests(tests, host);
  return 0;
}

void execute_tests(const json_t *tests, const char *host){
  int loop_var;
  int size;
  json_t *test;

  size = (int)json_array_size(test);
  for(loop_var = 0; loop_var < size; loop_var++){
    test = json_array_get(tests, loop_var);
    if(!json_is_object(test)){
      fprintf(stderr, "Test must be json object");
      exit(1);
    }
    run_test_plan(test, host);
  }
}

void run_test_plan(const json_t *test, const char *host){
  NetParams network_parameters;
  char *series_variable;
  char *x_axis_variable;
  json_t *series_variable_value;
  json_t *x_axis_variable_value;
  json_t *series;
  json_t *x_axis;
  json_t *series_values;
  json_t *x_axis_values;

  if(json_is_integer(json_object_get(test, "initcnwd"))){
    set_congestion_window(json_integer_value(json_object_get(test, "initcnwd")));
  }else{
    fprintf(stderr, "Congestion window must be a string\n");
    exit(1);
  }

  if(json_is_integer(json_object_get(test, "file_size_kb"))){
    set_send_size(json_integer_value(json_object_get(test, "initcnwd")));
  }else{
    fprintf(stderr, "Congestion window must be a string\n");
    exit(1);
  }

  network_parameters.latency = json_integer_value(json_object_get(test, "latency_ms"));
  network_parameters.bandwidth = json_integer_value(json_object_get(test, "bandwidth_kps"));
  network_parameters.packet_loss = json_integer_value(json_object_get(test, "packet_loss"));
  set_network_params(network_parameters);

  series = json_object_get(test, "series");
  x_axis = json_object_get(test, "x_axis");

  if(!json_is_object(series) || !json_is_object(x_axis)){
    fprintf(stderr, "x_axis and series must be json objects\n");
    exit(1);
  }

  series_values = json_object_get(series, "values");
  series_variable_value = json_object_get(series, "variable");
  x_axis_values = json_object_get(x_axis, "values");
  x_axis_variable_value = json_object_get(x_axis, "variable");

  if(!json_is_array(series_values) || !json_is_array(x_axis_values) || 
     !json_is_string(series_variable_value) || !json_is_string(x_axis_variable_value)){
    
  }
}

void set_network_params(const NetParams params, const char *host){
  char command[50];
  system("sudo ipfw -q flush");

  snprintf(command, 50, "sudo ipfw add pipe 1 ip from any to %s", host);
  system(command);

  snprintf(command, 50, "sudo ipfw pipe 1 config delay %dms bw %dKbit/s plr %f",
           params.latency, params.bandwidth, ((float)params.packet_loss)/100);
  system(command);
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




