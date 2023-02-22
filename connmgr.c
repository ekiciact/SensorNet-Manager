#ifndef __CONNMGR_H__
#define __CONNMGR_H__

#include "connmgr.h"

void *callback_copy(void *src_element) {
    pollinfo *copy = malloc(sizeof(pollinfo));
    MALLOC_ERR_HANDLER(copy == NULL, MALLOC_MEMORY_ERROR);
    copy->file_descriptors = ((pollinfo *) src_element)->file_descriptors;
    copy->last_record = ((pollinfo *) src_element)->last_record;
    copy->sensor_id = ((pollinfo *) src_element)->sensor_id;
    copy->socket = ((pollinfo *) src_element)->socket;
    return (void *) copy;
}

void callback_free(void **element) {
    free(*element);
}

int callback_compare(void *x, void *y) {
    return 0;
}

// Initializes the connection manager and starts listening for incoming connections on the specified port
void connmgr_listen(int port_number, sbuffer_t **sbuffer) {
    // Variables
    int server_socket, max_fd;
    fd_set master_set, read_set;
    struct timeval timeout;
    int count_total_values = 0;
    time_t last_activity;
    sensor_data_t *data;
    char *send_buf;
    dplist_t *sockets;

    // Initialize data and sockets list
    data = malloc(sizeof(*data));
    MALLOC_ERR_HANDLER(data == NULL, MALLOC_MEMORY_ERROR);
    sockets = dpl_create(callback_copy, callback_free, callback_compare);

    // Open write file
    FILE *fp = fopen("sensor_data_recv.txt", "w");

    // Create new socket in passive listening mode for the server
    create_server_socket(&server_socket, port_number);

    // Set last activity to determine when the server can be closed
    last_activity = time(NULL);

    // Initialize the master set and add the server socket
    FD_ZERO(&master_set);
    FD_SET(server_socket, &master_set);
    max_fd = server_socket;

    // Run through the loop as long as the server is active
    while (1) {
        read_set = master_set;
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        // Check for activity on the sockets
        int activity = select(max_fd + 1, &read_set, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (activity == 0) {
            close_inactive_sockets(sockets, &master_set, &max_fd, data, send_buf, &last_activity);
        } else {
            // Check for activity on the server socket
            if (FD_ISSET(server_socket, &read_set)) {
                handle_new_connection(server_socket, sockets, &master_set, &max_fd);
            }
            // Check for activity on the sensor sockets
            handle_sensor_data(sockets, &read_set, sbuffer, data, fp, &count_total_values);
        }
    }
}

void create_server_socket(int *server_socket, int port_number) {
    if (tcp_passive_open(server_socket, port_number) != TCP_NO_ERROR) {
        printf("Server can't be created\n");
        exit(EXIT_FAILURE);
    }
}

void close_inactive_sockets(dplist_t *sockets, fd_set *master_set, int *max_fd, sensor_data_t *data, char *send_buf,
                            time_t *last_activity) {
    // Create a copy of the master set
    fd_set copy_set;
    memcpy(&copy_set, master_set, sizeof(fd_set));

    // Iterate through all the sockets in the list
    for (int i = 0; i <= *max_fd; i++) {
        // Check if the current socket descriptor is in the set
        if (FD_ISSET(i, &copy_set)) {
            pollinfo *polldummy = (pollinfo *) dpl_get_element_at_index(sockets, i);

            // Close the socket if it has been inactive for more than TIMEOUT
            if ((polldummy->last_record + TIMEOUT) < time(NULL) && i != 0) {
                close_socket(polldummy, data, send_buf, i);
                *last_activity = time(NULL);
                break; // End the for loop because indexes don't line up anymore
            }
        }
    }
}

void handle_sensor_data(dplist_t *sockets, fd_set *read_set, sbuffer_t **sbuffer, sensor_data_t *data, FILE *fp,
                        int *count_total_values) {
    int i;
    for (i = 1; i < FD_SETSIZE; i++) {
        if (FD_ISSET(i, read_set)) {
            // retrieve the sensor data
            if (tcp_recv(i, data, sizeof(sensor_data_t)) != TCP_NO_ERROR) {
                printf("Error while receiving sensor data from socket %d\n", i);
                continue;
            }
            (*count_total_values)++;
            // print the sensor data to file
            fprintf(fp, "%"
            PRIu16
            " %g %ld\n", data->id, data->value, (long int) data->ts);
            // insert the sensor data into the buffer
            sbuffer_insert(*sbuffer, data);
            // update last_record timestamp in the polling list
            pollinfo *sensor = (pollinfo *) dpl_get_element_at_index(sockets, dpl_get_index_of_element(sockets, &i,
                                                                                                       &cmp_sensor_id));
            sensor->last_record = time(NULL);
        }
    }
}

void connmgr_free() {
    // Close all open sockets
    int i, max_fd = dpl_size(sockets);
    for (i = 1; i <= max_fd; i++) {
        pollinfo *polldummy = (pollinfo *) dpl_get_element_at_index(sockets, i);
        if (polldummy->socket > 0) {
            close(polldummy->socket);
        }
    }
    // Free all dynamically allocated memory
    dpl_free(&sockets, true);
    if (data != NULL) {
        free(data);
    }
    if (send_buf != NULL) {
        free(send_buf);
    }
}

void print_total_values() {
    printf("Total number of sensor data values processed: %d\n", count_total_values);
}

void create_server_socket(int *server_socket, int port_number) {
    if (tcp_passive_open(server_socket, port_number) != TCP_NO_ERROR) {
        printf("Server can't be created\n");
        exit(EXIT_FAILURE);
    }
}

#endif
