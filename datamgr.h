#ifndef DATAMGR_H_
#define DATAMGR_H_

#define _GNU_SOURCE

#include <stdio.h>
#include "config.h"
#include "sbuffer.h"
#include "main.h"
#include "datamgr.h"
#include "lib/dplist.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef SET_MAX_TEMP
#error SET_MAX_TEMP not set
#endif

#ifndef SET_MIN_TEMP
#error SET_MIN_TEMP not set
#endif

#ifndef TIMEOUT
#error TIMEOUT not specified!(in seconds)
#endif

#define NO_ERROR "c"
#define MEMORY_ERROR "b" // error due to mem alloc failure
#define INVALID_ERROR "a" //error due to sensor not found

//global variables
dplist_t *list;

//callback functions
void *element_copy(void *);

void element_free(void **);

int element_compare(void *, void *); //compares the ID of the element

//struct with information about each sensor
typedef struct sensors {
    uint16_t sensor_id;
    uint16_t room_id;
    double running_avg;
    time_t last_modified;
    double temperatures[RUN_AVG_LENGTH];
} sensor_t;

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    do { \
                      if (condition) { \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(EXIT_FAILURE); \
                      }    \
                    } while(0)

/**
 * Reads continiously all data from the shared buffer data structure, parse the room_id's
 * and calculate the running avarage for all sensor ids
 * When *buffer becomes NULL the method finishes. This method will NOT automatically free all used memory
 **/
void datamgr_parse_sensor_data(FILE *fp_sensor_map, sbuffer_t **buffer);

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified 
 * or datamgr_get_total_sensors will not return a valid result
 **/
void datamgr_free();

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
double datamgr_get_avg(sensor_id_t sensor_id);

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors();

#endif  //DATAMGR_H_
