#include "dplist.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// struct definition for the nodes in the list
struct dplist_node {
    void *element;
    dplist_node_t *prev;
    dplist_node_t *next;
};

// struct definition for the list
struct dplist {
    dplist_node_t *head;
    dplist_node_t *tail;
    int size;

    void *(*element_copy)(void *);

    void (*element_free)(void **);

    int (*element_compare)(void *, void *);
};

/* 
 * Create and allocate memory for a new list.
 *
 * element_copy: callback function to duplicate 'element'; If needed allocated new memory for the duplicated element.
 * element_free: callback function to free memory allocated to element
 * element_compare: callback function to compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y
 *
 * Returns a pointer to a newly-allocated and initialized list.
 */
dplist_t *dpl_create(
        void *(*element_copy)(void *element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list = malloc(sizeof(dplist_t));
    assert(list != NULL);
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

/*
 * Deletes all elements in the list.
 * Every list node of the list needs to be deleted. (free memory)
 * The list itself also needs to be deleted. (free all memory)
 * '*list' must be set to NULL.
 *
 * list: a double pointer to the list
 * free_element: if true call element_free() on the element of the list node to remove
 */
void dpl_free(dplist_t **list, bool free_element) {
    dplist_node_t *node = (*list)->head;
    while (node != NULL) {
        dplist_node_t *next_node = node->next;
        if (free_element) {
            (*list)->element_free(&node->element);
        }
        free(node);
        node = next_node;
    }
    free(*list);
    *list = NULL;
}

/*
 * Returns the number of elements in the list.
 * If 'list' is is NULL, -1 is returned.
 *
 * list: a pointer to the list
 *
 * Returns the size of the list.
 */
int dpl_size(dplist_t *list) {
    if (list == NULL) {
        return -1;
    }
    return list->size;
}

/*
 * Inserts a new list node containing an 'element' in the list at position 'index'.
 * the first list node has index 0.
 * If 'index' is 0 or negative, the list node is inserted at the start of 'list'.
 * If 'index' is bigger than the number of elements in the list, the list node is inserted at the end of the list.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the data that needs to be inserted
 * index: the position at which the element should be inserted in the list
 * insert_copy: if true use element_copy() to make a copy of 'element' and use the copy in the new list node, 
 *              otherwise the given element pointer is added to the list
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    if (list == NULL) {
        return NULL;
    }
    dplist_node_t *node = malloc(sizeof(dplist_node_t));
    assert(node != NULL);
    if (insert_copy) {
        node->element = list->element_copy(element);
    } else {
        node->element = element;
    }
    if (index <= 0) {
        // insert at start of list
        node->prev = NULL;
        node->next = list->head;
        if (list->head != NULL) {
            list->head->prev = node;
        } else {
            // list was empty
            list->tail = node;
        }
        list->head = node;
    } else if (index >= list->size) {
        // insert at end of list
        node->prev = list->tail;
        node->next = NULL;
        if (list->tail != NULL) {
            list->tail->next = node;
        } else {
            // list was empty
            list->head = node;
        }
        list->tail = node;
    } else {
        // insert at specific index in list
        dplist_node_t *curr_node = list->head;
        for (int i = 0; i < index - 1; i++) {
            curr_node = curr_node->next;
        }
        node->prev = curr_node;
        node->next = curr_node->next;
        curr_node->next->prev = node;
        curr_node->next = node;
    }
    list->size++;
    return list;
}

/*
 * Removes the list node at index 'index' from the list.
 * The list node itself should always be freed.
 * If 'index' is 0 or negative, the first list node is removed.
 * If 'index' is bigger than the number of elements in the list, the last list node is removed.
 * If the list is empty, return the unmodified list.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * index: the position at which the node should be removed from the list
 * free_element: if true, call element_free() on the element of the list node to remove
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    if (list == NULL || list->size == 0) {
        return NULL;
    }
    dplist_node_t *node;
    if (index <= 0) {
        // remove first node
        node = list->head;
        list->head = node->next;
        if (list->head == NULL) {
            // list is now empty
            list->tail = NULL;
        } else {
            list->head->prev = NULL;
        }
    } else if (index >= list->size) {
        // remove last node
        node = list->tail;
        list->tail = node->prev;
        if (list->tail == NULL) {
            // list is now empty
            list->head = NULL;
        } else {
            list->tail->next = NULL;
        }
    } else {
        // remove node at specific index
        node = list->head;
        for (int i = 0; i < index; i++) {
            node = node->next;
        }
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    if (free_element) {
        list->element_free(&node->element);
    }
    free(node);
    list->size--;
    return list;
}

/*
 * Returns a reference to the list node with index 'index' in the list.
 * If 'index' is 0 or negative, a reference to the first list node is returned.
 * If 'index' is bigger than the number of list nodes in the list, a reference to the last list node is returned.
 * If the list is empty, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * index: the position of the list node to return
 *
 * Returns a reference to the list node or NULL.
 */
dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL || list->size == 0) {
        return NULL;
    }
    if (index <= 0) {
        return list->head;
    } else if (index >= list->size) {
        return list->tail;
    } else {
        dplist_node_t *node = list->head;
        for (int i = 0; i < index; i++) {
            node = node->next;
        }
        return node;
    }
}

/*
 * Returns the element of the list node with index 'index' in the list.
 * If 'index' is 0 or negative, the element of the first list node is returned.
 * If 'index' is bigger than the number of list nodes in the list, the element of the last list node is returned.
 * If the list is empty, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * index: the position of the element to return
 *
 * Returns the element of the list node or NULL.
 */
void *dpl_get_element_at_index(dplist_t *list, int index) {
    dplist_node_t *node = dpl_get_reference_at_index(list, index);
    if (node == NULL) {
        return NULL;
    }
    return node->element;
}

/*
 * Sorts the list using the element_compare() function.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 *
 * Returns a pointer to the sorted list or NULL.
 */
dplist_t *dpl_sort(dplist_t *list) {
    if (list == NULL) {
        return NULL;
    }
    bool swapped = true;
    dplist_node_t *node;
    while (swapped) {
        swapped = false;
        node = list->head;
        while (node->next != NULL) {
            if (list->element_compare(node->element, node->next->element) > 0) {
                void *temp = node->element;
                node->element = node->next->element;
                node->next->element = temp;
                swapped = true;
            }
            node = node->next;
        }
    }
    return list;
}

/*
 * Returns a deep copy of the list.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 *
 * Returns a pointer to the deep copy of the list or NULL.
 */
dplist_t *dpl_copy(dplist_t *list) {
    if (list == NULL) {
        return NULL;
    }
    dplist_t *copy = dpl_create(list->element_copy, list->element_free, list->element_compare);
    dplist_node_t *node = list->head;
    while (node != NULL) {
        dpl_insert_at_index(copy, node->element, copy->size, true);
        node = node->next;
    }
    return copy;
}

/*
 * Returns the element at the start of the list.
 * If the list is empty, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 *
 * Returns the element at the start of the list or NULL.
 */
void *dpl_get_first_element(dplist_t *list) {
    return dpl_get_element_at_index(list, 0);
}

/*
 * Returns the element at the end of the list.
 * If the list is empty, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 *
 * Returns the element at the end of the list or NULL.
 */
void *dpl_get_last_element(dplist_t *list) {
    return dpl_get_element_at_index(list, -1);
}

/*
 * Inserts a new element at the start of the list.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the data that needs to be inserted
 * insert_copy: if true use element_copy() to make a copy of 'element' and use the copy in the new list node, 
 *              otherwise the given element pointer is added to the list
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_insert_first(dplist_t *list, void *element, bool insert_copy) {
    return dpl_insert_at_index(list, element, 0, insert_copy);
}

/*
 * Inserts a new element at the end of the list.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the data that needs to be inserted
 * insert_copy: if true use element_copy() to make a copy of 'element' and use the copy in the new list node, 
 *              otherwise the given element pointer is added to the list
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_insert_last(dplist_t *list, void *element, bool insert_copy) {
    return dpl_insert_at_index(list, element, -1, insert_copy);
}

/*
 * Removes the element at the start of the list.
 * If the list is empty, return the unmodified list.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * free_element: if true, call element_free() on the element of the list node to remove
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_remove_first(dplist_t *list, bool free_element) {
    return dpl_remove_at_index(list, 0, free_element);
}

/*
 * Removes the element at the end of the list.
 * If the list is empty, return the unmodified list.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * free_element: if true, call element_free() on the element of the list node to remove
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_remove_last(dplist_t *list, bool free_element) {
    return dpl_remove_at_index(list, -1, free_element);
}

/*
 * Returns the index of the first occurrence of 'element' in the list.
 * If 'element' is not found in the list, -1 is returned.
 * If 'list' is is NULL, -1 is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the element to search for in the list
 *
 * Returns the index of the element in the list or -1 if not found.
 */
int dpl_get_index_of_element(dplist_t *list, void *element) {
    if (list == NULL) {
        return -1;
    }
    dplist_node_t *node = list->head;
    int index = 0;
    while (node != NULL) {
        if (list->element_compare(node->element, element) == 0) {
            return index;
        }
        node = node->next;
        index++;
    }
    return -1;
}

/*
 * Returns a reference to the element at the given list node.
 * If 'reference' is NULL, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * reference: a pointer to the current list node
 *
 * Returns a reference to the element or NULL.
 */
void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL || reference == NULL) {
        return NULL;
    }
    return reference->element;
}


/*
 * Returns the reference to the first list node in the list.
 * If the list is empty, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 *
 * Returns the reference to the first list node in the list or NULL.
 */
dplist_node_t *dpl_get_first_reference(dplist_t *list) {
    if (list == NULL || list->size == 0) {
        return NULL;
    }
    return list->head;
}

/*
 * Returns the reference to the last list node in the list.
 * If the list is empty, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 *
 * Returns the reference to the last list node in the list or NULL.
 */
dplist_node_t *dpl_get_last_reference(dplist_t *list) {
    if (list == NULL || list->size == 0) {
        return NULL;
    }
    return list->tail;
}

/*
 * Returns the reference to the next list node after the given reference.
 * If the given reference is the last list node, NULL is returned.
 * If 'reference' is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * reference: a pointer to the current list node
 *
 * Returns the reference to the next list node or NULL.
 */
dplist_node_t *dpl_get_next_reference(dplist_t *list, dplist_node_t *reference) {
    if (reference == NULL) {
        return NULL;
    }
    return reference->next;
}

/*
 * Returns the reference to the previous list node before the given reference.
 * If the given reference is the first list node, NULL is returned.
 * If 'reference' is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * reference: a pointer to the current list node
 *
 * Returns the reference to the previous list node or NULL.
 */
dplist_node_t *dpl_get_previous_reference(dplist_t *list, dplist_node_t *reference) {
    if (reference == NULL) {
        return NULL;
    }
    return reference->prev;
}

/*
 * Returns the reference to the list node containing the given element.
 * If 'element' is not found in the list, NULL is returned.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the element to search for in the list
 *
 * Returns the reference to the list node containing the element or NULL.
 */
dplist_node_t *dpl_get_reference_of_element(dplist_t *list, void *element) {
    if (list == NULL) {
        return NULL;
    }
    dplist_node_t *node = list->head;
    while (node != NULL) {
        if (list->element_compare(node->element, element) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/*
 * Returns the index of the list node with the given reference.
 * If 'reference' is not found in the list, -1 is returned.
 * If 'list' is is NULL, -1 is returned.
 *
 * list: a pointer to the list
 * reference: a pointer to the list node
 *
 * Returns the index of the list node or -1 if not found.
 */
int dpl_get_index_of_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL) {
        return -1;
    }
    dplist_node_t *node = list->head;
    int index = 0;
    while (node != NULL) {
        if (node == reference) {
            return index;
        }
        node = node->next;
        index++;
    }
    return -1;
}

/*
 * Inserts a new list node containing an 'element' in the list after the given reference.
 * If 'reference' is NULL, the list node is inserted at the start of 'list'.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the data that needs to be inserted
 * reference: a pointer to the current list node
 * insert_copy: if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_insert_at_reference(dplist_t *list, void *element, dplist_node_t *reference, bool insert_copy) {
    if (list == NULL) {
        return NULL;
    }

    // Allocate memory for the new list node
    dplist_node_t *new_node = (dplist_node_t *) malloc(sizeof(dplist_node_t));
    assert(new_node != NULL);

    if (insert_copy) {
        new_node->element = list->element_copy(element);
    } else {
        new_node->element = element;
    }

    // Insert the new list node
    if (reference == NULL) {
        // Insert at the start of the list
        new_node->prev = NULL;
        new_node->next = list->head;
        if (list->head != NULL) {
            list->head->prev = new_node;
        }
        list->head = new_node;
    } else {
        // Insert after the given reference
        new_node->prev = reference;
        new_node->next = reference->next;
        if (reference->next != NULL) {
            reference->next->prev = new_node;
        }
        reference->next = new_node;
    }

    // Update the size of the list and the tail pointer if necessary
    list->size++;
    if (new_node->next == NULL) {
        list->tail = new_node;
    }

    return list;
}

/*
 * Inserts a new list node containing an 'element' in the list in a sorted manner.
 * The list must be sorted in ascending order.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the data that needs to be inserted
 * insert_copy: if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_insert_sorted(dplist_t *list, void *element, bool insert_copy) {
    if (list == NULL) {
        return NULL;
    }

    // Find the correct position to insert the new element
    dplist_node_t *node = list->head;
    while (node != NULL && list->element_compare(node->element, element) < 0) {
        node = node->next;
    }

    // Insert the new element
    return dpl_insert_at_reference(list, element, node, insert_copy);
}

/*
 * Removes the list node with the given reference from the list.
 * The list node itself should always be freed.
 * If 'reference' is NULL, the list is returned unmodified.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * reference: a pointer to the current list node
 * free_element: if true, call element_free() on the element of the list node to remove
 *
 * Returns a pointer to the list or NULL.
 */

/*
 * Removes the list node with the given reference from the list.
 * The list node itself should always be freed.
 * If 'reference' is NULL, the list is returned unmodified.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * reference: a pointer to the current list node
 * free_element: if true, call element_free() on the element of the list node to remove
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_remove_at_reference(dplist_t *list, dplist_node_t *reference, bool free_element) {
    if (list == NULL || reference == NULL) {
        return NULL;
    }

    // Update the pointers of the surrounding list nodes
    if (reference->prev != NULL) {
        reference->prev->next = reference->next;
    }
    if (reference->next != NULL) {
        reference->next->prev = reference->prev;
    }

    // Update the head and tail pointers if necessary
    if (reference == list->head) {
        list->head = reference->next;
    }
    if (reference == list->tail) {
        list->tail = reference->prev;
    }

    // Free the element if needed
    if (free_element) {
        list->element_free(&reference->element);
    }

    // Free the list node
    free(reference);

    // Update the size of the list
    list->size--;

    return list;
}


/*
 * Removes the list node containing the given element from the list.
 * The list node itself should always be freed.
 * If 'element' is not found in the list, the list is returned unmodified.
 * If 'list' is is NULL, NULL is returned.
 *
 * list: a pointer to the list
 * element: a pointer to the element to remove
 * free_element: if true, call element_free() on the element of the list node to remove
 *
 * Returns a pointer to the list or NULL.
 */
dplist_t *dpl_remove_element(dplist_t *list, void *element, bool free_element) {
    if (list == NULL) {
        return NULL;
    }

    // Find the list node containing the element
    dplist_node_t *reference = dpl_get_reference_of_element(list, element);
    if (reference == NULL) {
        return list;
    }

    // Remove the list node
    return dpl_remove_at_reference(list, reference, free_element);
}
