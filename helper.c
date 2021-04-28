#include "buffer.h"
#include <pthread.h>

// Creates a buffer with the given capacity
state_t* buffer_create(int capacity)
{
    state_t* buffer = (state_t*) malloc(sizeof(state_t));
    buffer->fifoQ = (fifo_t *) malloc ( sizeof (fifo_t));
    fifo_init(buffer->fifoQ,capacity);
    buffer->isopen = true;
    pthread_mutex_init(&(buffer->chmutex), NULL);
    pthread_mutex_init(&(buffer->chclose), NULL);
    pthread_cond_init(&(buffer->chconrec), NULL); //notempty
    pthread_cond_init(&(buffer->chconsend), NULL); //notfull
    //pthread_mutex_init(&(buffer->fifoQ->Qlock), NULL);
    return buffer;
}


// Writes data to the given buffer
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the buffer is full, the function waits till the buffer has space to write the new data
// Returns BUFFER_SUCCESS for successfully writing data to the buffer,
// CLOSED_ERROR if the buffer is closed, and
// BUFFER_ERROR on encountering any other generic error of any sort
enum buffer_status buffer_send(state_t *buffer, void* data)
{
    if(!buffer->isopen)
        {
            //pthread_mutex_unlock(&(buffer->chmutex));
            return CLOSED_ERROR;
        }

    pthread_mutex_lock(&(buffer->chmutex));
    int msg_size = get_msg_size(data);
    
    while(fifo_avail_size(buffer->fifoQ)<=msg_size){
        pthread_cond_wait(&(buffer->chconsend), &(buffer->chmutex));
        if(!buffer->isopen)
        {
            pthread_mutex_unlock(&(buffer->chmutex));
            return CLOSED_ERROR;
        }
    }
    buffer_add_Q(buffer,data);
    pthread_cond_broadcast(&(buffer->chconrec));
    pthread_mutex_unlock(&(buffer->chmutex));
    return BUFFER_SUCCESS;
}
// test_send_correctness 1
// Reads data from the given buffer and stores it in the function’s input parameter, data (Note that it is a double pointer).
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the buffer is empty, the function waits till the buffer has some data to read
// Return BUFFER_SPECIAL_MESSSAGE for successful retrieval of special data "splmsg"
// Returns BUFFER_SUCCESS for successful retrieval of any data other than "splmsg"
// CLOSED_ERROR if the buffer is closed, and
// BUFFER_ERROR on encountering any other generic error of any sort

enum buffer_status buffer_receive(state_t* buffer, void** data)
{
    
    int flag=0;
    
    if(!buffer->isopen)
    {
        //pthread_mutex_unlock(&(buffer->chmutex));
        return CLOSED_ERROR;
    }
    pthread_mutex_lock(&(buffer->chmutex));

    while(buffer->fifoQ->avilSize >= buffer->fifoQ->size){
        pthread_cond_wait(&(buffer->chconrec),&(buffer->chmutex));
        if(!buffer->isopen)
        {
            pthread_mutex_unlock(&(buffer->chmutex));
            return CLOSED_ERROR;
        }
    }
    buffer_remove_Q(buffer,data);

    pthread_cond_broadcast(&(buffer->chconsend));
    pthread_mutex_unlock(&(buffer->chmutex));

    if(strcmp(*(char**)(data),"splmsg") ==0)
    {
        flag=1;
    }
    if(flag==1){
        return BUFFER_SPECIAL_MESSSAGE;
    }
    else{
        return BUFFER_SUCCESS;
    }
    
    
}


// Closes the buffer and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the buffer is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns BUFFER_SUCCESS if close is successful,
// CLOSED_ERROR if the buffer is already closed, and
// BUFFER_ERROR in any other error case
enum buffer_status buffer_close(state_t* buffer)
{
    pthread_mutex_lock(&(buffer->chmutex));
    if(!buffer->isopen)
    {
        pthread_mutex_unlock(&(buffer->chmutex));
        return CLOSED_ERROR;
    }
    buffer->isopen = false;
    pthread_cond_broadcast(&(buffer->chconsend));
    pthread_cond_broadcast(&(buffer->chconrec));
    pthread_mutex_unlock(&(buffer->chmutex));
    return BUFFER_SUCCESS;
    
}

// Frees all the memory allocated to the buffer , using own version of sem flags
// The caller is responsible for calling buffer_close and waiting for all threads to finish their tasks before calling buffer_destroy
// Returns BUFFER_SUCCESS if destroy is successful,
// DESTROY_ERROR if buffer_destroy is called on an open buffer, and
// BUFFER_ERROR in any other error case

enum buffer_status buffer_destroy(state_t* buffer)
{
    if(buffer->isopen)
    {
        return DESTROY_ERROR;
    }
    pthread_mutex_destroy(&(buffer->chmutex));
    pthread_mutex_destroy(&(buffer->chclose));
    pthread_cond_destroy(&(buffer->chconrec));
    pthread_cond_destroy(&(buffer->chconsend));
    fifo_free(buffer->fifoQ);
    free(buffer);
    return BUFFER_SUCCESS;
}