#ifndef MAIN_H_
#define MAIN_H_

#define _GNU_SOURCE //needed for asprintf

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <poll.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "connmgr.h"
#include "sbuffer.h"
#include "config.h"
#include "datamgr.h"
#include "sensor_db.h"
#include "errmacros.h"

#ifndef TIMEOUT
#error TIMEOUT not specified!(in seconds)
#endif

//global variables
pthread_t threads[3];
sbuffer_t *sbuffer;
pthread_mutex_t fifolock;

/*
* This method handles the conmgr
*/
void *start_conmgr(void *port);

/*
* This method handles the datamgr
*/
void *start_datamgr(void *arg);

/*
* This method handles the storagemgr 
and tries to connect DB three times if first try failed.
It finished gateway process when three connection tries failed
*/
void *start_storagemgr(void *arg);

/*
* main checks if the user has given a port and creates the child process
*/
int main(int argc, char *argv[]);

/*
* This method handles child process
* Reads log messages from FIFO as a child process
and write them to gateway.log file
*/
void run_child(void);

/*
* This method handles the parent process
*/
void run_parent(char *argv[]);

/*
 * This method will end parent and child process
 */
void terminate();

/*
 * Print the help message
 */
void print_help(void);

/*
 * initialise the fifomgr
 */
void fifomgr_init();

/*
 * read the fifomgr
 */
void fifomgr_read();

/*
 * write to the fifomgr
 */
void fifomgr_write(char *text);

#endif  //MAIN_H_
