#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

int balance = 1000;
sem_t lock;

typedef struct {
    int id;
    int amount;
} thread_data;

void* unsafe_withdraw(void* arg) {
    thread_data* data = (thread_data*)arg;
    int current = balance;
    printf("Luong %d: doc %d, rut %d\n", data->id, current, data->amount);
    
    usleep(rand() % 300000 + 200000);
    
    if(current >= data->amount) {
        balance = current - data->amount;
        printf("Luong %d rut %d → con %d\n", data->id, data->amount, balance);
    }
    
    return NULL;
}

void* safe_withdraw(void* arg) {
    thread_data* data = (thread_data*)arg;
    
    sem_wait(&lock);
    int current = balance;
    printf("Luong %d: doc %d, rut %d\n", data->id, current, data->amount);
    
    usleep(rand() % 300000 + 200000);
    
    if(current >= data->amount) {
        balance = current - data->amount;
        printf("Luong %d rut %d → con %d\n", data->id, data->amount, balance);
    }
    sem_post(&lock);
    
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t t1, t2;
    
    printf("--Khong dong bo--\n");
    balance = 1000;
    
    thread_data data1 = {1, rand() % 300 + 200};
    thread_data data2 = {2, rand() % 300 + 200};
    
    pthread_create(&t1, NULL, unsafe_withdraw, &data1);
    pthread_create(&t2, NULL, unsafe_withdraw, &data2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("So du: %d\n\n", balance);
    
    printf("--Co dong bo--\n");
    balance = 1000;
    sem_init(&lock, 0, 1);
    
    thread_data data3 = {1, rand() % 300 + 200};
    thread_data data4 = {2, rand() % 300 + 200};
    
    pthread_create(&t1, NULL, safe_withdraw, &data3);
    pthread_create(&t2, NULL, safe_withdraw, &data4);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    sem_destroy(&lock);
    
    printf("So du: %d\n", balance);
    
    return 0;
}