#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <math.h>
#include <jansson.h>

#include "client.h"
#include "string.h"
#include "server.h"

int main() { 
  json_t *configuration;
  const char *host;
  json_t *tests;
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
  json_decref(tests);
  return 0;
}

void execute_tests(const json_t *tests, const char *host){
  int loop_var;
  int size;
  json_t *test;

  size = (int)json_array_size(tests);
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
  int x_axis_loop;
  int series_loop;
  int size;
  int iterations;
  json_t *series_variable_value;
  json_t *x_axis_variable_value;
  json_t *series;
  json_t *x_axis;
  json_t *series_values;
  json_t *x_axis_values;
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
    fprintf(stderr, "axis or series defined incorrectly");
  }

  iterations = json_integer_value(json_object_get(test, "iterations"));
  
  fprintf(stderr, "series: %i, x: %i\n", json_array_size(series_values), json_array_size(x_axis_values));
  for(series_loop = 0; series_loop < json_array_size(series_values); series_loop++){
    set_defaults(test, &network_parameters, &size);
    set_independent_variable(json_string_value(series_variable_value), json_integer_value(json_array_get(series_values, series_loop)), network_parameters, &size);
    for(x_axis_loop = 0; x_axis_loop < json_array_size(x_axis_values); x_axis_loop++){
      set_independent_variable(json_string_value(x_axis_variable_value), json_integer_value(json_array_get(x_axis_values, x_axis_loop)), network_parameters, &size);
      read_from_server(iterations, size);
    }
  }
}

void set_defaults(const json_t *test, NetParams *params, int *size){
  if(json_is_integer(json_object_get(test, "initcnwd"))){
    set_congestion_window(json_integer_value(json_object_get(test, "initcnwd")));
  }else{
    fprintf(stderr, "Congestion window must be a string\n");
    exit(1);
  }

  if(json_is_integer(json_object_get(test, "file_size_kb"))){
    set_send_size((*size = json_integer_value(json_object_get(test, "file_size_kb"))));
  }else{
    fprintf(stderr, "Congestion window must be a string\n");
    exit(1);
  }

  params->latency = json_integer_value(json_object_get(test, "latency_ms"));
  params->bandwidth = json_integer_value(json_object_get(test, "bandwidth_kps"));
  params->packet_loss = json_integer_value(json_object_get(test, "packet_loss"));
  set_network_params(*params);
}

void set_independent_variable(const char *var_name, const int value, NetParams params, int *size){
  fprintf(stderr, "var name: %s, value: %d\n", var_name, value);
  if(strncmp(var_name, "initcnwd", 8) == 0){
    fprintf(stderr, "Set congestion window\n");
    set_congestion_window(value);
  }else if(strncmp(var_name, "bandwidth_kps", 13) == 0){
    set_bandwidth(value, params);
  }else if(strncmp(var_name, "latency_ms", 9) == 0){
    set_latency(value, params);
  }else if(strncmp(var_name, "packet_loss", 11) == 0){
    set_packet_loss(value, params);
  }else if(strncmp(var_name, "file_size_kb", 12) == 0){
    set_send_size(value);
    *size = value;
  }
}

void set_latency(const int latency, NetParams params){
  params.latency = latency;
  set_network_params(params);
}

void set_bandwidth(const int bandwidth, NetParams params){
  params.bandwidth = bandwidth;
  set_network_params(params);
}

void set_packet_loss(const int packet_loss, NetParams params){
  params.packet_loss = packet_loss;
  set_network_params(params);
}

void set_network_params(const NetParams params){
  char command_buffer[100];
  snprintf(command_buffer, 100, "sudo tc qdisc del dev eth0 root");
  system(command_buffer);
  system("sudo tc qdisc add dev eth0 root handle 1: htb default 12");


  snprintf(command_buffer, 100, "sudo tc class add dev eth0 parent 1:1 classid 1:12 htb rate %dkbps ceil %dkbps", params.bandwidth, params.bandwidth);
  fprintf(stderr, "%s\n", command_buffer);
  system(command_buffer);

  snprintf(command_buffer, 100, "sudo tc qdisc add dev eth0 parent 1:12 netem delay %dms loss %f%%", params.latency, ((float)params.packet_loss)/1000);
  fprintf(stderr, "%s\n", command_buffer);
  system(command_buffer);
}

void read_from_server(int iterations, int size){
  int server_socket_fd;
  int bytes_sent;
  int bytes_recieved;
  int loop_var;
  char *recieve_buffer;
  struct sockaddr_in external_address;
  struct timespec tps, tpe;
  long *time_differences;
  long mean_time;
  double stdev_time;

  time_differences = malloc((iterations + 1) * sizeof(long));

  recieve_buffer = malloc(size * 1024);

  external_address.sin_family = AF_INET;
  external_address.sin_port = htons(5010);
  external_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(&(external_address.sin_zero), '\0', 8);

  mean_time = 0;
  for(loop_var = 0; loop_var < iterations; loop_var++){
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &tps) != 0){
      perror("clock_gettime");
      exit(1);
    }
    if((server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
      perror("socket");
      exit(1);
    }

    if (connect(server_socket_fd, (struct sockaddr *)&external_address, sizeof(struct sockaddr)) == -1) {
      perror("connect");
      exit(1);
    }

    if((bytes_sent = send(server_socket_fd, SEND_DATA, 8, 0)) == -1){
      perror("send");
      exit(1);
    }

    if ((bytes_recieved=recv(server_socket_fd, recieve_buffer, size * 1024, MSG_WAITALL)) == -1) {
      perror("recv");
      exit(1);
    }

    close(server_socket_fd);
    if(clock_gettime(CLOCK_MONOTONIC_RAW, &tpe) !=0){
      perror("clock_gettime");
      exit(1);
    }

    time_differences[loop_var] = ((((unsigned long long)tpe.tv_sec) * 1000) + (((unsigned long long)tpe.tv_nsec) / 1000000) -
		    (((unsigned long long)tps.tv_sec) * 1000) - (((unsigned long long)tps.tv_nsec) / 1000000));
  }

  mean_time = 0;
  stdev_time = 0;

  for(loop_var = 0; loop_var < iterations; loop_var++){
    mean_time += time_differences[loop_var];
  }
  mean_time /= iterations;

  for(loop_var = 0; loop_var < iterations; loop_var++){
    stdev_time += (time_differences[loop_var] - mean_time) * (time_differences[loop_var] - mean_time);
  }
  stdev_time /= iterations - 1;
  stdev_time = sqrt(stdev_time);
  free(time_differences);
  free(recieve_buffer);

  fprintf(stderr, "Mean time: %ld, Standard deviation time: %f\n\n", mean_time, stdev_time);
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
  fprintf(stderr, "Command: %s\n", command);

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

  configuration = (String*)new_string();
  configuration_file = fopen("./configuration.json", "r");

  while(fgets(line, 500, configuration_file) != NULL){
    cat_string(configuration, line, 500);
  }
  fclose(configuration_file);  /* close the file prior to exiting the routine */

  root = json_loads(configuration->buffer, 0, &error);
  free_string(configuration);
  return root;
}




