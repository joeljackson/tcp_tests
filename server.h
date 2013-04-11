#ifndef _SERVER_H
#define _SERVER_H

//Simple communication protocol
//"SETCWND #{size}" -> Sets the size of the congestion window on the server
//"SENDDAT #{num}" -> Sends num buffers from the server to the client
//"SETSIZE #{size}" -> Sets the size of the buffer to be sent from the server to the client


const int INCOMING_BUFFER_SIZE = 1024;
const char SET_CONGESTION_WINDOW[8] = "SETCWND";
const char SEND_DATA[8] = "SENDDAT";
const char SET_DATA_SIZE[8] = "SETSIZE";

int create_socket();
void set_congestion_window(int window_size);
int set_download_size(int kilobytes, char *transmission_window);
void send_data(int send_socket, char *data, int size);

#endif
