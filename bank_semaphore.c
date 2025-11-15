#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int balance_unsafe = 1000;
int balance_safe = 1000;
sem_t balance_sem;

void print_operation(const char* type, int thread_id, const char* action, int amount, int balance) {
    printf("%-8s T%d: %s %3d → Balance: %4d\n", type, thread_id, action, amount, balance);
}

// ==================== UNSAFE VERSION ====================
void* unsafe_operation(void* arg) {
    int* params = (int*)arg;
    int withdraw_amount = params[0];
    int deposit_amount = params[1];
    int thread_id = params[2];
    
    // Đọc số dư hiện tại
    int current = balance_unsafe;
    printf("UNSAFE   T%d: Read balance = %d\n", thread_id, current);
    
    // ĐỘ TRỄ LỚN - tạo điều kiện cho race condition
    usleep(800000);
    
    // Thực hiện cả rút và nạp (nguy hiểm!)
    if (current >= withdraw_amount) {
        balance_unsafe = current - withdraw_amount;
        print_operation("UNSAFE", thread_id, "Withdrew", withdraw_amount, balance_unsafe);
    }
    
    usleep(500000);
    
    // Nạp tiền
    current = balance_unsafe; // Đọc lại - có thể đã bị thay đổi!
    balance_unsafe = current + deposit_amount;
    print_operation("UNSAFE", thread_id, "Deposited", deposit_amount, balance_unsafe);
    
    return NULL;
}

// ==================== SAFE VERSION ====================
void* safe_operation(void* arg) {
    int* params = (int*)arg;
    int withdraw_amount = params[0];
    int deposit_amount = params[1];
    int thread_id = params[2];
    
    sem_wait(&balance_sem);
    
    // Đọc số dư hiện tại (được bảo vệ)
    int current = balance_safe;
    printf("SAFE     T%d: Read balance = %d\n", thread_id, current);
    
    usleep(800000);
    
    // Thực hiện cả rút và nạp (AN TOÀN)
    if (current >= withdraw_amount) {
        balance_safe = current - withdraw_amount;
        print_operation("SAFE", thread_id, "Withdrew", withdraw_amount, balance_safe);
    }
    
    usleep(500000);
    
    // Nạp tiền (vẫn trong critical section)
    current = balance_safe;
    balance_safe = current + deposit_amount;
    print_operation("SAFE", thread_id, "Deposited", deposit_amount, balance_safe);
    
    sem_post(&balance_sem);
    return NULL;
}

void run_unsafe_demo() {
    printf("\n🔴 ===== UNSAFE DEMO - RACE CONDITION GUARANTEED =====\n");
    printf("Start balance: %d\n", balance_unsafe);
    printf("Both threads: Withdraw 600 → Deposit 400\n\n");
    
    balance_unsafe = 1000;
    pthread_t t1, t2;
    
    int params1[3] = {600, 400, 1}; // withdraw, deposit, thread_id
    int params2[3] = {600, 400, 2};
    
    pthread_create(&t1, NULL, unsafe_operation, params1);
    pthread_create(&t2, NULL, unsafe_operation, params2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    int expected = 1000 - 600 + 400 - 600 + 400; // = 600
    printf("\n📊 UNSAFE RESULTS:\n");
    printf("Final balance: %d\n", balance_unsafe);
    printf("Expected balance: %d\n", expected);
    
    if (balance_unsafe != expected) {
        printf("❌ RACE CONDITION CONFIRMED! Data corrupted by %d\n", 
               balance_unsafe - expected);
    }
}

void run_safe_demo() {
    printf("\n🟢 ===== SAFE DEMO - SEMAPHORE PROTECTION =====\n");
    printf("Start balance: %d\n", balance_safe);
    printf("Both threads: Withdraw 600 → Deposit 400\n\n");
    
    balance_safe = 1000;
    sem_init(&balance_sem, 0, 1);
    
    pthread_t t1, t2;
    
    int params1[3] = {600, 400, 1};
    int params2[3] = {600, 400, 2};
    
    pthread_create(&t1, NULL, safe_operation, params1);
    pthread_create(&t2, NULL, safe_operation, params2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    sem_destroy(&balance_sem);
    
    int expected = 1000 - 600 + 400 - 600 + 400; // = 600
    printf("\n📊 SAFE RESULTS:\n");
    printf("Final balance: %d\n", balance_safe);
    printf("Expected balance: %d\n", expected);
    
    if (balance_safe == expected) {
        printf("✅ DATA CONSISTENCY MAINTAINED!\n");
    }
}

void multiple_runs_demo() {
    printf("\n🔄 ===== MULTIPLE RUNS COMPARISON =====\n");
    
    int unsafe_runs[5], safe_runs[5];
    
    for (int i = 0; i < 5; i++) {
        // Unsafe run
        balance_unsafe = 1000;
        pthread_t t1, t2;
        int params1[3] = {600, 400, 1};
        int params2[3] = {600, 400, 2};
        
        pthread_create(&t1, NULL, unsafe_operation, params1);
        pthread_create(&t2, NULL, unsafe_operation, params2);
        
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        
        unsafe_runs[i] = balance_unsafe;
        
        // Safe run
        balance_safe = 1000;
        sem_init(&balance_sem, 0, 1);
        
        pthread_create(&t1, NULL, safe_operation, params1);
        pthread_create(&t2, NULL, safe_operation, params2);
        
        pthread_join(t1, NULL);
        pthread_join(t2, NULL);
        
        sem_destroy(&balance_sem);
        safe_runs[i] = balance_safe;
    }
    
    printf("\n📈 COMPARISON RESULTS (5 runs):\n");
    printf("Run # | Unsafe Result | Safe Result | Consistent?\n");
    printf("------|---------------|-------------|------------\n");
    
    for (int i = 0; i < 5; i++) {
        printf("  %d   |      %4d     |    %4d     |    %s\n", 
               i + 1, unsafe_runs[i], safe_runs[i],
               (unsafe_runs[i] == 600) ? "YES" : "NO ❌");
    }
    
    printf("\n💡 Observation: Unsafe results vary, Safe results are always consistent!\n");
}

int main() {
    printf("=========================================\n");
    printf("   FINAL RACE CONDITION DEMONSTRATION\n");
    printf("   Clear Comparison: UNSAFE vs SAFE\n");
    printf("=========================================\n");
    
    run_unsafe_demo();
    run_safe_demo();
    multiple_runs_demo();
    
    printf("\n=========================================\n");
    printf("   KEY FINDINGS:\n");
    printf("   • UNSAFE: Inconsistent results\n");
    printf("   • SAFE: Always correct\n"); 
    printf("   • Semaphore = Data consistency\n");
    printf("=========================================\n");
    
    return 0;
}