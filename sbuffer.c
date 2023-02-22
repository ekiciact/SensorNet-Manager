/**
 * \author Mustafa Ekici
 */

#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;

    // initialize mutex and condition variable
    if (pthread_mutex_init(&(*buffer)->mutex, NULL) != 0) {
        return SBUFFER_FAILURE;
    }
    if (pthread_cond_init(&(*buffer)->cond_var, NULL) != 0) {
        return SBUFFER_FAILURE;
    }

    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }

    // destroy mutex and condition variable
    if (pthread_mutex_destroy(&(*buffer)->mutex) != 0) {
        return SBUFFER_FAILURE;
    }
    if (pthread_cond_destroy(&(*buffer)->cond_var) != 0) {
        return SBUFFER_FAILURE;
    }

    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}


int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;

    // acquire mutex
    pthread_mutex_lock(&(buffer->mutex));

    if (buffer->head == NULL) {
        // release mutex and return SBUFFER_NO_DATA
        pthread_mutex_unlock(&(buffer->mutex));
        return SBUFFER_NO_DATA;
    }

    *data = buffer->head->data;
    dummy = buffer->head;
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        buffer->head = buffer->tail = NULL;
    } else  // buffer has many nodes empty
    {
        buffer->head = buffer->head->next;
    }
    free(dummy);

    // release mutex
    pthread_mutex_unlock(&(buffer->mutex));

    return SBUFFER_SUCCESS;
}


int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;

    // lock the mutex before inserting data into the buffer
    pthread_mutex_lock(&buffer->mutex);

    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }

    // unlock the mutex after inserting data into the buffer
    pthread_mutex_unlock(&buffer->mutex);

    return SBUFFER_SUCCESS;
}

int sbuffer_get_data(sbuffer_t *buffer, sensor_data_t *data) {
    if (buffer == NULL) return SBUFFER_FAILURE;

    // Lock the buffer against writer threads
    pthread_mutex_lock(&buffer_mutex);

    // Check if there is any data available in the buffer
    if (buffer->head == NULL) {
        pthread_mutex_unlock(&buffer_mutex);
        return SBUFFER_NO_DATA;
    }

    // Read the data from the buffer
    *data = buffer->head->data;

    // Unlock the buffer
    pthread_mutex_unlock(&buffer_mutex);

    return SBUFFER_SUCCESS;
}
