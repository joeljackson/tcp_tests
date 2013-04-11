#include <stdio.h>

#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "server.h"


int main() {
  int incoming_socket_file_descriptor;
  int incoming_socket_connection_file_descriptor;
  int sin_size;
  int bytes_recieved;
  int size_argument;
  int kilobytes_to_send = 14;
  struct sockaddr_in incoming_address;
  char incoming_buffer[INCOMING_BUFFER_SIZE];

  char *to_transmit = malloc(kilobytes_to_send * 1024);
  memset(to_transmit, 'A', kilobytes_to_send * 1024);

  incoming_socket_file_descriptor = create_socket();

  while(1) {
    sin_size = sizeof(struct sockaddr_in);
    if((incoming_socket_connection_file_descriptor = accept( incoming_socket_file_descriptor, (struct sockaddr*) &incoming_address, &sin_size)) == -1){
      perror("accept");
      exit(1);
    }

    fprintf(stderr, "About to recv data\n");
    if((bytes_recieved=recv(incoming_socket_connection_file_descriptor, incoming_buffer, INCOMING_BUFFER_SIZE, 0)) == -1) {
      perror("recv");
      exit(1);
    }
    fprintf(stderr, "Recieved %d bytes: %s\n", bytes_recieved, incoming_buffer);

    if(bytes_recieved > 8){
      size_argument = atoi((char*)(incoming_buffer + 8));
    }

    if(strncmp(incoming_buffer, SET_CONGESTION_WINDOW, 7) == 0){
      fprintf(stderr, "Set Congestion Window\n");
      set_congestion_window(size_argument);
    }else if(strncmp(incoming_buffer, SEND_DATA, 7) == 0){
      fprintf(stderr, "Send Data\n");
      send_data(incoming_socket_connection_file_descriptor, to_transmit, kilobytes_to_send);
    }else if(strncmp(incoming_buffer, SET_DATA_SIZE, 7) == 0){
      fprintf(stderr, "Set download size\n");
      kilobytes_to_send = set_download_size(size_argument, to_transmit);
    }

    close(incoming_socket_connection_file_descriptor);
  }

  return 0;
}
 
int set_download_size(int size, char *transmission_window){
  free(transmission_window);
  transmission_window = malloc(size * 1024);
  memset(transmission_window, 'A', size * 1024);  
  return size;
}

void set_congestion_window(int size){
  char command[200];
  snprintf(command, 200, "sudo ip route change default via 10.40.162.1 dev eth0 initcwnd %d", size);
  system(command);
}

void send_data(int send_socket, char *data, int size){
  int bytes_sent;
  int tcp_info_length;
  struct tcp_info tcp_info;

  if((bytes_sent = send(send_socket, data, size * 1024, 0)) == -1){
    perror("send");
    exit(1);
  }

  tcp_info_length = sizeof(tcp_info);
  if( getsockopt( send_socket, SOL_TCP, TCP_INFO, (void *)&tcp_info, (socklen_t *)&tcp_info_length ) == 0 ) {
    fprintf(stderr, "snd_cwnd: %u,snd_ssthresh: %u,rcv_ssthresh: %u,rtt: %u,rtt_var: %u\n",
	   tcp_info.tcpi_snd_cwnd,
	   tcp_info.tcpi_snd_ssthresh,
	   tcp_info.tcpi_rcv_ssthresh,
	   tcp_info.tcpi_rtt,
	   tcp_info.tcpi_rttvar
	   );
  }else{
    perror("getsockopt");
    exit(1);
  }
  fprintf(stderr, "Bytes sent %i\n", bytes_sent);
  fprintf(stderr, "tcp initcwnd:%u\n", tcp_info.tcpi_snd_cwnd);
}

int create_socket(){
  int incoming_socket_file_descriptor;
  struct sockaddr_in my_address;

  if((incoming_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
    perror("socket");
    exit(1);
  }

  int yes = 1;
  if( setsockopt(incoming_socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ){
    perror("setsockopt");
    exit(1);
  }

  my_address.sin_family = AF_INET;
  my_address.sin_port = htons(5010);
  my_address.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(&(my_address.sin_zero), '\0', 8);

  if((bind(incoming_socket_file_descriptor, (struct sockaddr*)&my_address, sizeof(struct sockaddr))) == -1){
    perror("bind");
    exit(1);
  }

  if(listen(incoming_socket_file_descriptor, 2) == -1){
    perror("listen");
    exit(1);
  }
  
  return incoming_socket_file_descriptor;
}
