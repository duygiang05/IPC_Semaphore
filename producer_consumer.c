#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

sem_t empty, full, mutex;

void print_buffer() {
    printf("Buffer: [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if(buffer[i] != 0 ) printf("%d", buffer[i]);
        else printf("_");
        if (i < BUFFER_SIZE - 1) printf(" ");
    }
    printf("]\n");
}

void* producer(void* arg) {
    int id = *(int*)arg;
    int item;
    
    for (int i = 0; i < 5; i++) {
        item = rand() % 100 + 1;
        
        sem_wait(&empty);
        sem_wait(&mutex);
        
        buffer[in] = item;
        printf("Producer %d: %d at %d - ", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        print_buffer();
        
        sem_post(&mutex);
        sem_post(&full);
        
        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    int id = *(int*)arg;
    int item;
    
    for (int i = 0; i < 5; i++) {
        sem_wait(&full);
        sem_wait(&mutex);
        
        item = buffer[out];
        buffer[out] = 0;
        printf("Consumer %d: %d from %d - ", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        print_buffer();
        
        sem_post(&mutex);
        sem_post(&empty);
        
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t prod[2], cons[2];
    int prod_id[2] = {1, 2};
    int cons_id[2] = {1, 2};
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 0;
    }
    
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
    
    pthread_create(&prod[0], NULL, producer, &prod_id[0]);
    pthread_create(&prod[1], NULL, producer, &prod_id[1]);
    pthread_create(&cons[0], NULL, consumer, &cons_id[0]);
    pthread_create(&cons[1], NULL, consumer, &cons_id[1]);
    
    pthread_join(prod[0], NULL);
    pthread_join(prod[1], NULL);
    pthread_join(cons[0], NULL);
    pthread_join(cons[1], NULL);
    
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    return 0;
}