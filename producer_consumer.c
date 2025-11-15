#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5

// Biến toàn cục
int buffer[BUFFER_SIZE];
int in = 0, out = 0;

// Semaphore
sem_t empty;
sem_t full; 
sem_t mutex;

void* producer(void* arg) {
    int producer_id = *(int*)arg;
    int item;
    
    for (int i = 0; i < 5; i++) {
        item = rand() % 100 + 1;  // Tạo item ngẫu nhiên 1-100
        
        sem_wait(&empty);     // Chờ nếu buffer đầy
        sem_wait(&mutex);     // Vào critical section
        
        // Sản xuất item
        buffer[in] = item;
        printf("🏭 Producer %d produced: %d at position %d\n", 
               producer_id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        
        // In trạng thái buffer
        printf("📦 Buffer: [");
        for (int j = 0; j < BUFFER_SIZE; j++) {
            if (buffer[j] != 0) {
                printf("%d", buffer[j]);
            } else {
                printf("_");
            }
            if (j < BUFFER_SIZE - 1) printf(", ");
        }
        printf("]\n");
        
        sem_post(&mutex);     // Ra khỏi critical section
        sem_post(&full);      // Tăng số ô đầy
        
        sleep(rand() % 2 + 1);  // Nghỉ ngẫu nhiên 1-2 giây
    }
    
    printf("✅ Producer %d finished\n", producer_id);
    return NULL;
}

void* consumer(void* arg) {
    int consumer_id = *(int*)arg;
    int item;
    
    for (int i = 0; i < 5; i++) {
        sem_wait(&full);      // Chờ nếu buffer rỗng
        sem_wait(&mutex);     // Vào critical section
        
        // Tiêu thụ item
        item = buffer[out];
        buffer[out] = 0;  // Đánh dấu ô đã trống
        printf("👨‍💼 Consumer %d consumed: %d from position %d\n", 
               consumer_id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        
        // In trạng thái buffer
        printf("📦 Buffer: [");
        for (int j = 0; j < BUFFER_SIZE; j++) {
            if (buffer[j] != 0) {
                printf("%d", buffer[j]);
            } else {
                printf("_");
            }
            if (j < BUFFER_SIZE - 1) printf(", ");
        }
        printf("]\n");
        
        sem_post(&mutex);     // Ra khỏi critical section  
        sem_post(&empty);     // Tăng số ô trống
        
        sleep(rand() % 3 + 1);  // Nghỉ ngẫu nhiên 1-3 giây
    }
    
    printf("✅ Consumer %d finished\n", consumer_id);
    return NULL;
}

int main() {
    printf("=================================\n");
    printf("   PRODUCER-CONSUMER DEMO\n");
    printf("   Using Semaphore for IPC\n");
    printf("=================================\n\n");
    
    pthread_t producers[2], consumers[2];
    int producer_ids[2] = {1, 2};
    int consumer_ids[2] = {1, 2};
    
    // Khởi tạo buffer rỗng
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 0;
    }
    
    // Khởi tạo semaphore
    sem_init(&empty, 0, BUFFER_SIZE);  // Ban đầu buffer trống
    sem_init(&full, 0, 0);             // Ban đầu không có ô đầy
    sem_init(&mutex, 0, 1);            // Binary semaphore
    
    printf("🔄 Initializing...\n");
    printf("📊 Buffer size: %d\n", BUFFER_SIZE);
    printf("🎯 Each producer/consumer will handle 5 items\n\n");
    
    // Tạo các thread
    pthread_create(&producers[0], NULL, producer, &producer_ids[0]);
    pthread_create(&producers[1], NULL, producer, &producer_ids[1]);
    pthread_create(&consumers[0], NULL, consumer, &consumer_ids[0]);
    pthread_create(&consumers[1], NULL, consumer, &consumer_ids[1]);
    
    // Chờ tất cả thread hoàn thành
    pthread_join(producers[0], NULL);
    pthread_join(producers[1], NULL);
    pthread_join(consumers[0], NULL);
    pthread_join(consumers[1], NULL);
    
    // Hủy semaphore
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    printf("\n=================================\n");
    printf("   ALL OPERATIONS COMPLETED! 🎉\n");
    printf("=================================\n");
    
    return 0;
}