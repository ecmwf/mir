#include "utils.h"
#include <stdio.h>
#include <pthread.h>

#include "../src/ecregrid_api.h"


typedef struct fd_node fd_node;

struct fd_node
{
    int index;
    field_description* fd;
    struct fd_node* next;
};

static fd_node* fd_handles = 0;

static pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

int _get_handle_from_field_description(field_description* fd)
{
    pthread_mutex_lock(&mutex_);

    int rtn = 0;

    struct fd_node* nodePoint = fd_handles;
    while (nodePoint)
    {
        if (nodePoint->fd == fd)
        {
            rtn = nodePoint->index;
            break;
        }

        nodePoint = nodePoint->next;
    }
    
    pthread_mutex_unlock(&mutex_);
    return rtn;
}

void display_list(struct fd_node* llist) {
 if (llist) 
 {
 while(llist->next != NULL) {
  printf("%d \n", llist->index);
  llist = llist->next;
 }

 printf("%d\n", llist->index);
 }
}

int _add_field_description(field_description* fd)
{
    pthread_mutex_lock(&mutex_);
    
    struct fd_node* newItem = 0;
    int hiIndex = 0; // find highest handle index value

    if (!fd_handles)
    {
        // nothing there yet
        fd_handles = (struct fd_node*)malloc(sizeof(struct fd_node));
        newItem = fd_handles;
    }
    else
    {
        // search for the end of the list
        //
        struct fd_node* lastItem = fd_handles;

        while (lastItem->next) {

            lastItem= lastItem->next;
            newItem = lastItem->next;
        }

        // we're now at the end. append new item
        lastItem->next = (struct fd_node*)malloc(sizeof(struct fd_node));
        hiIndex = lastItem->index;

        newItem = lastItem->next;
    }

    // add a new item to the end
    //newItem = (struct fd_node*)malloc(sizeof(struct fd_node));
    newItem->index = hiIndex+1;
    newItem->fd = fd;
    newItem->next = 0;

    int rtn = newItem->index;
    pthread_mutex_unlock(&mutex_);
    
    return rtn;
}

field_description* _get_field_description_from_handle(int handle)
{
    pthread_mutex_lock(&mutex_);

    struct fd_node* nodePoint = fd_handles;
    field_description* rtn = 0;

    while(nodePoint)
    {
        if (nodePoint->index == handle)
        {
            rtn = nodePoint->fd;
            break;
        }

        nodePoint = nodePoint->next;
    }

    pthread_mutex_unlock(&mutex_);
    return rtn;
}

field_description* _remove_field_description(int handle)
{
    // rtns pointer to object that is removed from list but not deleted
    pthread_mutex_lock(&mutex_);

    struct fd_node* nodePoint = fd_handles;
    struct fd_node* prevPoint = 0;
    field_description* rtn = 0;
    int found = 0;

    while (nodePoint)
    {
        if (nodePoint->index == handle)
        {
            found = 1;
            break;
        }
        prevPoint = nodePoint;
        nodePoint = nodePoint->next;
    }

    if (found)
    {
  
        rtn = nodePoint->fd;

        if (prevPoint)
            prevPoint->next = nodePoint->next;
        else
            fd_handles = nodePoint->next;

        free(nodePoint);
        nodePoint = 0;
    }

    pthread_mutex_unlock(&mutex_);
    return rtn;

}

grib_handle* _ecregrid_util_local_grib_handle_from_message ( char* data, size_t buflen )
{
    /* The zero in the arguments picks up the default context which seems to
     * be what is done routinely elsewhere */
    return grib_handle_new_from_message(0, data, buflen);
}

int _ecregrid_util_message_from_local_grib_handle(grib_handle* h, const void **msg, size_t *size) 
{
    return grib_get_message(h,msg,size);
}

grib_handle* _ecregrid_util_vector_grib_handle_get_at(vector_grib_handle* vec, size_t index)
{
    if (vec)
    {
        if (index == 0)
            return vec->one;
        if (index == 1)
            return vec->two;
    }
    return 0;
}
/*
int ecregrid_grib_c_get_message(grib_handle* h, const void **msg, size_t *size) {
    return grib_get_message(h,msg,size);
}
*/
