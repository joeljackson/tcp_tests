#include <jansson.h>

#ifndef _CLIENT_H
#define _CLIENT_H

json_t* read_configuration();

void set_congestion_window(int packets);
void set_send_size(int kilobytes);
void read_from_server(int iterations, int size);

#endif