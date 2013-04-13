#include <jansson.h>

#ifndef _CLIENT_H
#define _CLIENT_H

typedef struct NetParams{
  int latency;
  int bandwidth;
  int packet_loss;
} NetParams;

json_t* read_configuration();

void set_congestion_window(int packets);
void set_send_size(int kilobytes);
void read_from_server(int iterations, int size);
void execute_tests(const json_t *tests, const char *host);
void run_test_plan(const json_t *test, const char *host);
void set_network_params(const NetParams params);
void set_independent_variable(const char *var_name, const int value, 
			      const NetParams params, int *size);
void set_latency(const int latency, const NetParams params);
void set_bandwidth(const int bandwidth, const NetParams params);
void set_packet_loss(const int packet_loss, const NetParams params);

#endif
