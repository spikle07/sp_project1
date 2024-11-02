/*
 * CS 551 Project "Memory manager".
 * You do not need to turn in this file.	
 */

#include "memory_manager.h"
#include "interposition.h"

void test_basic_allocation() {
    printf("\nTesting basic allocation:\n");
    void* ptr1 = malloc(8);  // Should use 16-byte slot
    void* ptr2 = malloc(16); // Should use 16-byte slot
    mem_mngr_print_snapshot();
    
    printf("\nFreeing memory:\n");
    free(ptr1);
    free(ptr2);
    mem_mngr_print_snapshot();
}

void test_multiple_allocations() {
    printf("\nTesting multiple allocations:\n");
    void* ptrs[9];  // One more than MEM_BATCH_SLOT_COUNT
    
    // Fill first batch
    printf("\nFilling first batch:\n");
    for (int i = 0; i < MEM_BATCH_SLOT_COUNT; i++) {
        ptrs[i] = malloc(16);
        printf("Allocated block %d at %p\n", i, ptrs[i]);
    }
    
    printf("\nFirst batch filled, adding one more allocation:\n");
    // This should create a new batch
    ptrs[8] = malloc(16);
    printf("Allocated block 8 at %p (should be in new batch)\n", ptrs[8]);
    
    mem_mngr_print_snapshot();
    
    printf("\nFreeing memory from first batch:\n");
    for (int i = 0; i < MEM_BATCH_SLOT_COUNT; i++) {
        printf("Freeing block %d\n", i);
        free(ptrs[i]);
    }
    
    printf("\nFreeing memory from second batch:\n");
    printf("Freeing block 8\n");
    free(ptrs[8]);
    
    mem_mngr_print_snapshot();
}

void test_error_cases() {
    printf("\nTesting error cases:\n");
    
    printf("1. Testing double free:\n");
    void* ptr = malloc(16);
    printf("Allocated ptr = %p\n", ptr);
    printf("First free (should succeed):\n");
    free(ptr);
    printf("Second free (should fail):\n");
    free(ptr);
    
    printf("\n2. Testing NULL pointer free:\n");
    free(NULL);
    
    printf("\n3. Testing oversized allocation:\n");
    void* big_ptr = malloc(100);  // Should fail (> 5x alignment)
    if (big_ptr == NULL) {
        printf("Successfully rejected oversized allocation\n");
    }
    
    mem_mngr_print_snapshot();
}

int main(int argc, char* argv[]) {
    printf("Starting Memory Manager Tests\n");
    printf("============================\n");

    mem_mngr_init();
    test_basic_allocation();
    mem_mngr_leave();
    
    printf("\nResetting for multiple allocation test\n");
    mem_mngr_init();
    test_multiple_allocations();
    mem_mngr_leave();
    
    printf("\nResetting for error case test\n");
    mem_mngr_init();
    test_error_cases();
    mem_mngr_leave();

    printf("\nTests completed.\n");
    return 0;
}