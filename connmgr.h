#ifndef CONNMGR_H_
#define CONNMGR_H_

#include <poll.h>
#include <stdio.h>
#include "lib/tcpsock.h"
#include "lib/dplist.h"
#include "config.h"
#include "sbuffer.h"
#include "errmacros.h"
#include "main.h"

typedef struct pollfd filedescr;

typedef struct {
    filedescr file_descriptors;
    time_t last_record;
    sensor_id_t sensor_id;
    tcpsock_t *socket;
} pollinfo;

#ifndef TIMEOUT
#error TIMEOUT not specified!(in seconds)
#endif

// callback functions
void *callback_copy(void *src_element);

void callback_free(void **element);

int callback_compare(void *x, void *y);

/*

This method holds the core functionality of your connmgr. It starts listening on the given port and
when when a sensor node connects it writes the data to a sensor_data_recv file. This file must have the
same format as the sensor_data file in assignment 6 and 7.
*/
void connmgr_listen(int port_number, sbuffer_t **sbuffer);

/*
This method should be called to clean up the connmgr, and to free all used memory.
After this no new connections will be accepted
*/

void connmgr_free();

/*

Close the socket and send the information to the log file, print the information in terminal if DDEBUG is defined
*/
void close_inactive_sockets(dplist_t *sockets, fd_set *master_set, int *max_fd, sensor_data_t data);

/*

Log the total amount of values. Print them in terminal if DDEBUG is defined
*/

void print_total_values(int count_total_values);

/*

Handle new connections
*/

void handle_new_connection(int server_socket, dplist_t *sockets, fd_set *master_set, int *max_fd);

#endif  
