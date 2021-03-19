#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "blocking_q.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#define TODO printf("TODO!\n");

/**
 * Internal function to blocking_q. Takes an element
 * in the queue. This functions assumes the following
 * preconditions:
 *  - The thread has safe access to the queue
 *  - The queue is NOT empty
 * Also update the size.
 * @param q the queue
 * @return an element
 */
task_ptr __blocking_q_take(blocking_q *q) { // NOLINT(bugprone-reserved-identifier)

    task_ptr first_element = q->first->data;

    blocking_q_node *new_head = q->first->next;
    free(q->first);
    q->first = new_head;
    q->sz--;

    return first_element;
}

/**
 * Create a blocking queue. Initializes the synchronisation primitives
 * and
 * @param q the queue
 * @return if init was successful.
 */
bool blocking_q_init(blocking_q *q) {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int ret_lock = pthread_mutex_init(&lock, NULL);
    int ret_cond = pthread_cond_init(&cond, NULL);
    if (ret_lock!=0 || ret_cond!=0){
        return false;
    }
    blocking_q_node *first = NULL;

    q->lock = lock;
    q->cond = cond;
    q->first = first;
    q->sz = 0;
    return true;
}

/**
 * Destroy a blocking queue. Removes the allocations of the data
 * and destroys the sync. primitives.
 * @param q ptr to the blocking queue
 */
void blocking_q_destroy(blocking_q *q) {
    pthread_mutex_destroy(&(q->lock));
    pthread_cond_destroy(&(q->cond));
    blocking_q_node *current = q->first;
    while (current != NULL && current->next !=NULL){
        task_ptr t = __blocking_q_take(q);
        //free(t);
        current = q->first;
    }
}

/**
 * Put a task in the blocking queue. This task can fail if no
 * memory is available to allocate a new entry in the queue
 * @param q the queue
 * @param data the data description to put inside the queue
 * @returns if the data was put correctly inside the queue.
 */
bool blocking_q_put(blocking_q *q, task_ptr data) {
    blocking_q_node *new = malloc(sizeof (blocking_q_node));
    if(!new){
        return false;
    }
    new->data = data;
    new->next = NULL;
    pthread_mutex_lock(&q->lock);
    blocking_q_node *parent = q->first;
    int count = 1;

    if(parent == NULL){
        q->first = new;
    }
    else{
        blocking_q_node *current = parent->next;

        while(current!= NULL){
            parent = current;
            current = parent->next;

            count++;
        }

        parent->next = new;
        count++;
    }

    q->sz = count;

    pthread_cond_signal(&(q->cond));
    pthread_mutex_unlock(&q->lock);
    return true;
}

/**
 * Get an element in the blocking queue. If the queue is empty,
 * the current thread is put to sleep until an element is added
 * to the queue.
 * @param q the blocking queue
 * @return the element
 */
task_ptr blocking_q_get(blocking_q *q) {
//    pthread_mutex_lock(&q->lock);
    while(q->sz == 0){
        pthread_cond_wait(&q->cond, &q->lock);
//        pthread_mutex_lock(&q->lock);
    }
    task_ptr val = __blocking_q_take(q);
//    pthread_mutex_unlock(&q->lock);
    return val;
}

/**
 * Drain as many elements as possible into the area allowed
 * by the pointer. This function does not block.
 * @param q the queue
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @return the number of entries written.
 */
size_t blocking_q_drain(blocking_q *q, task_ptr *data, size_t sz) {
    size_t counter = 0;
    pthread_mutex_lock(&q->lock);
    blocking_q_node *current = q->first;
    task_ptr task;
    while(current!= NULL && counter < sz){
        current = current->next;
        task = __blocking_q_take(q);
        data[counter] = task;
        counter++;
    }
    pthread_mutex_unlock(&q->lock);
    return counter;
}

/**
 * Drain at least min elements in the buffer. This function
 * might block if there are not enough elements to drain.
 * @param q the queue
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @param min the minimum amounts of elements to drain (must be less than sz)
 * @return the number of elements written
 */
size_t blocking_q_drain_at_least(blocking_q *q, task_ptr *data, size_t sz, size_t min) {
    int count = 0;
    while(count < sz){
        blocking_q_node *current = q->first;
        if(count < min){
            data[count] = blocking_q_get(q);
        } else {
            if(current == NULL){
                break;
            } else {
                data[count] = __blocking_q_take(q);
            }
        }
        count++;
    }
    return count;
}

/**
 * Check the first element in the queue. This will allocate storage for a copy
 * of the character. If the allocation fails, this function returns false.
 * @param q the queue
 * @param c pointer to a pointer where an allocated char will be stored
 * @return if there is an element allocated in the pointer
 */
bool blocking_q_peek(blocking_q *q, task **c) {
    task *temp = malloc(sizeof(task*));
    if(!temp){
        return false;
    }
    *c = temp;
    blocking_q_node *current = q->first;
    if(current == NULL){
        return false;
    }
    void *ret = memcpy(*c, q->first->data, sizeof(task));
    if(!ret){
        return false;
    }
    return true;
}