#include "memory_manager.h"
#include "interposition.h"
#include "common.h"
#include <assert.h>

// Helper function to print test headers
void print_test_header(const char* test_name) {
    printf("\n=== Running Test: %s ===\n", test_name);
}

// Helper function to print test results
void print_test_result(const char* test_name, int success) {
    printf("%s: %s\n", test_name, success ? "PASS" : "FAIL");
}

// Helper function to print bitmap contents
void print_bitmap(unsigned char* bitmap, int size) {
    for (int byte = 0; byte < size; byte++) {
        for (int bit = 0; bit < 8; bit++) {
            printf("%d", (bitmap[byte] >> bit) & 0x01);
            if ((byte * 8 + bit + 1) % 4 == 0) printf(" ");
        }
    }
    printf("\n");
}

// ================== Bitmap Tests ==================

void test_bitmap_operations() {
    print_test_header("Bitmap Operations");
    
    // Test 1: Basic find_first_bit
    unsigned char bitmap1[] = {0xF7, 0xFF}; // 1111 0111 1111 1111
    printf("Initial bitmap: ");
    print_bitmap(bitmap1, sizeof(bitmap1));
    
    int pos = bitmap_find_first_bit(bitmap1, sizeof(bitmap1), 0);
    print_test_result("Find first zero bit (expect 3)", pos == 3);
    
    // Test 2: Set bit
    int result = bitmap_set_bit(bitmap1, sizeof(bitmap1), 3);
    print_test_result("Set bit 3", result == BITMAP_OP_SUCCEED);
    
    // Test 3: Check if bit is set
    int is_set = bitmap_bit_is_set(bitmap1, sizeof(bitmap1), 3);
    print_test_result("Check bit 3 is set", is_set == 1);
    
    // Test 4: Clear bit
    result = bitmap_clear_bit(bitmap1, sizeof(bitmap1), 3);
    print_test_result("Clear bit 3", result == BITMAP_OP_SUCCEED);
    
    // Test 5: Error cases
    result = bitmap_find_first_bit(NULL, 2, 0);
    print_test_result("NULL pointer handling", result == BITMAP_OP_ERROR);
    
    result = bitmap_set_bit(bitmap1, 0, 0);
    print_test_result("Zero size handling", result == BITMAP_OP_ERROR);
}

// ================== Memory Manager Tests ==================

void test_basic_allocation() {
    print_test_header("Basic Allocation");
    
    void* ptr1 = malloc(16);
    assert(ptr1 != NULL);
    mem_mngr_print_snapshot();
    
    free(ptr1);
    mem_mngr_print_snapshot();
}

void test_multiple_allocations() {
    print_test_header("Multiple Allocations");
    
    void* ptrs[10];
    for(int i = 0; i < 10; i++) {
        ptrs[i] = malloc(16);
        assert(ptrs[i] != NULL);
    }
    mem_mngr_print_snapshot();
    
    for(int i = 0; i < 10; i++) {
        free(ptrs[i]);
    }
    mem_mngr_print_snapshot();
}

void test_different_sizes() {
    print_test_header("Different Sizes");
    
    void* ptr1 = malloc(16);  // Minimum size
    void* ptr2 = malloc(32);  // 2x alignment
    void* ptr3 = malloc(64);  // 4x alignment
    void* ptr4 = malloc(80);  // 5x alignment (maximum)
    
    assert(ptr1 != NULL);
    assert(ptr2 != NULL);
    assert(ptr3 != NULL);
    assert(ptr4 != NULL);
    
    mem_mngr_print_snapshot();
    
    free(ptr1);
    free(ptr2);
    free(ptr3);
    free(ptr4);
}

void test_error_cases() {
    print_test_header("Error Cases");
    
    // Test allocation of size 0
    void* ptr1 = malloc(0);
    print_test_result("Allocation size 0", ptr1 == NULL);
    
    // Test allocation beyond 5x alignment
    void* ptr2 = malloc(96);
    print_test_result("Allocation beyond limit", ptr2 == NULL);
    
    // Test double free
    void* ptr3 = malloc(16);
    free(ptr3);
    printf("Testing double free (expect error message):\n");
    free(ptr3);
    
    // Test invalid pointer
    printf("Testing invalid pointer free (expect error message):\n");
    free((void*)0x12345678);
}

void test_batch_expansion() {
    print_test_header("Batch Expansion");
    
    void* ptrs[20];  // More than MEM_BATCH_SLOT_COUNT (8)
    
    for(int i = 0; i < 20; i++) {
        ptrs[i] = malloc(16);
        assert(ptrs[i] != NULL);
    }
    
    mem_mngr_print_snapshot();
    
    for(int i = 0; i < 20; i++) {
        free(ptrs[i]);
    }
}

void test_mixed_operations() {
    print_test_header("Mixed Operations");
    
    // Allocate different sizes
    void* ptr1 = malloc(16);
    void* ptr2 = malloc(32);
    void* ptr3 = malloc(48);
    assert(ptr1 != NULL && ptr2 != NULL && ptr3 != NULL);
    
    mem_mngr_print_snapshot();
    
    // Free middle allocation and reallocate
    free(ptr2);
    void* new_ptr = malloc(32);
    assert(new_ptr != NULL);
    
    mem_mngr_print_snapshot();
    
    // Clean up
    free(ptr1);
    free(ptr3);
    free(new_ptr);
}

int main() {
    printf("\n====== Starting Test Suite ======\n");
    
    // First run bitmap tests
    printf("\n=== Bitmap Tests ===\n");
    test_bitmap_operations();
    
    // Then run memory manager tests
    printf("\n=== Memory Manager Tests ===\n");
    mem_mngr_init();
    
    test_basic_allocation();
    test_multiple_allocations();
    test_different_sizes();
    test_error_cases();
    test_batch_expansion();
    test_mixed_operations();
    
    mem_mngr_leave();
    
    printf("\n====== Test Suite Completed ======\n");
    return 0;
}