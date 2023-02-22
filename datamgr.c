#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "datamgr.h"

dplist_t *list;
pthread_mutex_t list_mutex;

void *element_copy(void *data) {
    sensor_t *sensor_data = (sensor_t *) data;
    sensor_t *copy = malloc(sizeof(sensor_t));
    memcpy(copy, sensor_data, sizeof(sensor_t));
    return copy;
}

void element_free(void **data) {
    free(*data);
    *data = NULL;
}

int element_compare(void *x, void *y) {
    sensor_t *sensor_x = (sensor_t *) x;
    sensor_t *sensor_y = (sensor_t *) y;
    if (sensor_x->sensor_id < sensor_y->sensor_id) return -1;
    else if (sensor_x->sensor_id > sensor_y->sensor_id) return 1;
    else return 0;
}

void datamgr_parse_sensor_data(FILE *fp_sensor_map, sbuffer_t **buffer) {
    //initialize list_mutex with 
    pthread_mutex_init(&list_mutex, NULL);

    list = dpl_create(element_copy, element_free, element_compare);
    // all the above callback functions that I have provided you before, these function use the dplist.h library,
    // they will be used to make deep copy, free the elements and compare the elements of the list

    // read sensor information from file and create list of sensors
    int room_id;
    uint16_t sensor_id;
    while (fscanf(fp_sensor_map, "%"SCNu16
    ",%d", &sensor_id, &room_id) == 2) {
        sensor_t *sensor = malloc(sizeof(sensor_t));
        ERROR_HANDLER(sensor == NULL, "malloc() error");
        sensor->sensor_id = sensor_id;
        sensor->room_id = room_id;
        sensor->running_avg = 0.0;
        sensor->last_modified = 0;
        memset(sensor->temperatures, 0, sizeof(double) * RUN_AVG_LENGTH);
        pthread_mutex_lock(&list_mutex);
        dpl_insert_at_index(list, sensor, dpl_size(list), true);
        pthread_mutex_unlock(&list_mutex);
    }

    sensor_data_t sensor_data;
    while (*buffer) {
        // read sensor data from buffer
        int status = sbuffer_remove(*buffer, &sensor_data);
        if (status == SBUFFER_SUCCESS) {
            // find corresponding sensor in list
            pthread_mutex_lock(&list_mutex);
            sensor_t search = {sensor_data.id};
            int index = dpl_get_index_of_element(list, &search);
            if (index != -1) {
                sensor_t *sensor = (sensor_t *) dpl_get_element_at_index(list, index);
                if (sensor) {
                    // update sensor data
                    sensor->last_modified = sensor_data.ts;
                    memmove(&sensor->temperatures[1], &sensor->temperatures[0], sizeof(double) * (RUN_AVG_LENGTH - 1));
                    sensor->temperatures[0] = sensor_data.value;
                    double sum = 0.0;
                    for (int i = 0; i < RUN_AVG_LENGTH; i++) {
                        sum += sensor->temperatures[i];
                    }
                    sensor->running_avg = sum / RUN_AVG_LENGTH;
                }
            }
            pthread_mutex_unlock(&list_mutex);
        }
    }
}

void datamgr_free() {
    dpl_free(&list, true);
    pthread_mutex_destroy(&list_mutex);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id) {
    //find corresponding sensor in list
    pthread_mutex_lock(&list_mutex);
    sensor_t search = {sensor_id};
    int index = dpl_get_index_of_element(list, &search);
    uint16_t room_id = -1;
    if (index != -1) {
        sensor_t *sensor = (sensor_t *) dpl_get_element_at_index(list, index);
        if (sensor) {
            room_id = sensor->room_id;
        }
    }
    pthread_mutex_unlock(&list_mutex);
    return room_id;
}

double datamgr_get_avg(sensor_id_t sensor_id) {
    //find corresponding sensor in list
    pthread_mutex_lock(&list_mutex);
    sensor_t search = {sensor_id};
    int index = dpl_get_index_of_element(list, &search);
    double avg = 0.0;
    if (index != -1) {
        sensor_t *sensor = (sensor_t *) dpl_get_element_at_index(list, index);
        if (sensor) {
            avg = sensor->running_avg;
        }
    }
    pthread_mutex_unlock(&list_mutex);
    return avg;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    //find corresponding sensor in list
    pthread_mutex_lock(&list_mutex);
    sensor_t search = {sensor_id};
    int index = dpl_get_index_of_element(list, &search);
    time_t last_modified = 0;
    if (index != -1) {
        sensor_t *sensor = (sensor_t *) dpl_get_element_at_index(list, index);
        if (sensor) {
            last_modified = sensor->last_modified;
        }
    }
    pthread_mutex_unlock(&list_mutex);
    return last_modified;
}

int datamgr_get_total_sensors() {
    int total_sensors;
    pthread_mutex_lock(&list_mutex);
    total_sensors = dpl_size(list);
    pthread_mutex_unlock(&list_mutex);
    return total_sensors;
}
