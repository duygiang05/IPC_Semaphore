#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5

int buf[N], in = 0, out = 0;
sem_t empty, full, mutex;

void in_buf() {
    printf("Buf: [");
    for(int i = 0; i < N; i++) {
        if (buf[i] != 0 ) printf("%d", buf[i]);
        else printf("_");
        if(i < N-1) printf(" ");
    }
    printf("]\n");
}

void* sx(void* arg) {
    int id = *(int*)arg;
    for(int i = 0; i < 3; i++) {
        int sp = rand() % 100;
        sem_wait(&empty);
        sem_wait(&mutex);
        buf[in] = sp;
        printf("SX%d: +%d ind: %d - ", id, sp, in);
        in_buf();
        in = (in + 1) % N;
        sem_post(&mutex);
        sem_post(&full);
        sleep(1);
    }
    return NULL;
}

void* td(void* arg) {
    int id = *(int*)arg;
    for(int i = 0; i < 3; i++) {
        sem_wait(&full);
        sem_wait(&mutex);
        int sp = buf[out];
        buf[out] = 0;
        printf("TD%d: -%d ind: %d - ", id, sp, out);
        in_buf();
        out = (out + 1) % N;
        sem_post(&mutex);
        sem_post(&empty);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t p[2], c[2];
    int id[2] = {1, 2};
    
    sem_init(&empty, 0, N);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
    
    for(int i = 0; i < N; i++) buf[i] = 0;
    
    pthread_create(&p[0], NULL, sx, &id[0]);
    pthread_create(&p[1], NULL, sx, &id[1]);
    pthread_create(&c[0], NULL, td, &id[0]);
    pthread_create(&c[1], NULL, td, &id[1]);
    
    pthread_join(p[0], NULL);
    pthread_join(p[1], NULL);
    pthread_join(c[0], NULL);
    pthread_join(c[1], NULL);
    
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    return 0;
}