#include "utils.h"

typedef struct fifo_element {
    void *data;
    size_t size;
    struct fifo_element *next;
} fifo_element_t;

typedef struct fifo_struct {
    fifo_element_t *first;
    fifo_element_t *last;
    fifo_element_t *crnt;
    int num_elements;
} fifo_struct_t;

/*
    Create a new FIFO data structure.
*/
fifo_t fifo_create(void) {
    MARK();
    fifo_struct_t *fs;

    if(NULL  == (fs = (fifo_struct_t*)calloc(1, sizeof(fifo_struct_t))))
        fatal_error("cannot allocate memory for FIFO struct");

    return (fifo_t)fs;
}

/*
    Destroy the FIFO. This must be done to free the memory. The get function
    does not free any memory.
*/
void fifo_destroy(fifo_t fifo) {
    MARK();
    fifo_struct_t *fs = (fifo_struct_t *)fifo;
    fifo_element_t *crnt, *next;

    if(fs != NULL) {
        for(crnt = fs->first; crnt != NULL; crnt = next) {
            next = crnt->next;
            if(crnt->data != NULL)
                free(crnt->data);
            free(crnt);
        }
        free(fs);
    }
}

/*
    Add an element to the FIFO.
*/
void fifo_add(fifo_t fifo, void *data, size_t size) {
    MARK();
    fifo_struct_t *fs = (fifo_struct_t *)fifo;
    fifo_element_t *nelem;

    if(fs != NULL) {
        if(NULL == (nelem = (fifo_element_t*)calloc(1, sizeof(fifo_element_t))))
            fatal_error("cannot allocate memory for FIFO element");

        // malloc can return a zero length buffer
        if(NULL == (nelem->data = malloc(size)))
            fatal_error("cannot allocate memory for FIFO element data");

        if(data != NULL) {
            // memcpy may not like zero length buffers in some implementations
            memcpy(nelem->data, data, size);
        }
        nelem->size = size;

        if(fs->first == NULL) {
            fs->first = nelem;
            fs->last = nelem;
            fs->crnt = nelem;
        }
        else {
            fs->last->next = nelem;
            fs->last = nelem;
        }
    }
    else
        fatal_error("attempt to add to an invalid FIFO");
}

/*
    Copy the data into the buffer supplied and advance the crnt pointer to
    the next element. If the data parameter is NULL, the pointer is advanced
    without copying the data.
*/
int fifo_get(fifo_t fifo, void *data, size_t size) {
    MARK();
    fifo_struct_t *fs = (fifo_struct_t *)fifo;

    if(fs != NULL) {
        if(fs->crnt != NULL) {
            if(fs->crnt->data != NULL)
                if(data != NULL)
                    memcpy(data, fs->crnt->data, size);
            // the position in the FIFO has been advanced, even if no data was
            // copied.
            fs->crnt = fs->crnt->next;
            return 1;
        }
    }

    return 0; // fail or at the end of the list
}

/*
    Reset the crnt pointer to the beginning of the list.
*/
int fifo_reset(fifo_t fifo) {
    MARK();
    fifo_struct_t *fs = (fifo_struct_t *)fifo;
    if(fs != NULL) {
        fs->crnt = fs->first;
        return 1;
    }
    else
        return 0; // fail
}
