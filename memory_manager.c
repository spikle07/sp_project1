/*
 * CS 551 Project "Memory manager".
 * This file needs to be turned in.	
 */

#include "memory_manager.h"

static STRU_MEM_LIST* mem_pool = NULL;

/*
 * Print out the current status of the memory manager.
 * Reading this function may help you understand how the memory manager organizes the memory.
 * Do not change the implementation of this function. It will be used to help the grading.
 */
void mem_mngr_print_snapshot(void)
{
    STRU_MEM_LIST * mem_list = NULL;

    printf("============== Memory snapshot ===============\n");

    mem_list = mem_pool; // Get the first memory list
    while(NULL != mem_list)
    {
        STRU_MEM_BATCH * mem_batch = mem_list->first_batch; // Get the first mem batch from the list 

        printf("mem_list %p slot_size %d batch_count %d free_slot_bitmap %p\n", 
                   mem_list, mem_list->slot_size, mem_list->batch_count, mem_list->free_slots_bitmap);
        bitmap_print_bitmap(mem_list->free_slots_bitmap, mem_list->bitmap_size);

        while (NULL != mem_batch)
        {
            printf("\t mem_batch %p batch_mem %p\n", mem_batch, mem_batch->batch_mem);
            mem_batch = mem_batch->next_batch; // get next mem batch
        }

        mem_list = mem_list->next_list;
    }

    printf("==============================================\n");
}

// helper to create a new memory batch
static STRU_MEM_BATCH* create_memory_batch(int slot_size) {
    STRU_MEM_BATCH* batch = (STRU_MEM_BATCH*)malloc(sizeof(STRU_MEM_BATCH));
    if (!batch) return NULL;

    // allocate memory for batch slots
    batch->batch_mem = malloc(slot_size * MEM_BATCH_SLOT_COUNT);
    if (!batch->batch_mem) {
        free(batch);
        return NULL;
    }

    batch->next_batch = NULL;
    return batch;
}
// helper to create a new memory list
static STRU_MEM_LIST* create_memory_list(int slot_size) {
    STRU_MEM_LIST* new_list = (STRU_MEM_LIST*)malloc(sizeof(STRU_MEM_LIST));
    if (!new_list) return NULL;

    // Calculate initial bitmap size 
    int bitmap_size = (MEM_BATCH_SLOT_COUNT + BIT_PER_BYTE - 1) / BIT_PER_BYTE;
    new_list->free_slots_bitmap = (unsigned char*)malloc(bitmap_size);
    if (!new_list->free_slots_bitmap) {
        free(new_list);
        return NULL;
    }

    // init bitmap using for loop
    for (int i = 0; i < bitmap_size; i++) {
        new_list->free_slots_bitmap[i] = 0xFF;
    }

    new_list->slot_size = slot_size;
    new_list->batch_count = 1;
    new_list->bitmap_size = bitmap_size;
    new_list->next_list = NULL;

    new_list->first_batch = create_memory_batch(slot_size);
    if (!new_list->first_batch) {
        free(new_list->free_slots_bitmap);
        free(new_list);
        return NULL;
    }

    return new_list;
}

/*
 * Initialize the memory manager with 16 bytes(defined by the macro MEM_ALIGNMENT_BOUNDARY) slot size mem_list.
 * Initialize this list with 1 batch of slots.
 */
void mem_mngr_init(void) {
    // Initialize with default 16-byte aligned memory list
    mem_pool = create_memory_list(MEM_ALIGNMENT_BOUNDARY);
}

/*
 * Clean up the memory manager (e.g., release all the memory allocated)
 */
void mem_mngr_leave(void) {
    STRU_MEM_LIST* curr_list = mem_pool;
    
    while (curr_list) {
        STRU_MEM_LIST* next_list = curr_list->next_list;
        STRU_MEM_BATCH* curr_batch = curr_list->first_batch;
        
        // Free all batches in this list
        while (curr_batch) {
            STRU_MEM_BATCH* next_batch = curr_batch->next_batch;
            free(curr_batch->batch_mem);
            free(curr_batch);
            curr_batch = next_batch;
        }
        
        // Free the bitmap and list structure
        free(curr_list->free_slots_bitmap);
        free(curr_list);
        curr_list = next_list;
    }
    
    mem_pool = NULL;
}

static void expand_bitmap(STRU_MEM_LIST* list) {
    int new_bitmap_size = list->bitmap_size + (MEM_BATCH_SLOT_COUNT + BIT_PER_BYTE - 1) / BIT_PER_BYTE;
    unsigned char* new_bitmap = (unsigned char*)malloc(new_bitmap_size);
    
    // Copy old bitmap content
    memcpy(new_bitmap, list->free_slots_bitmap, list->bitmap_size);
    
    // Initialize new portion
    memset(new_bitmap + list->bitmap_size, 0xFF, new_bitmap_size - list->bitmap_size);
    
    free(list->free_slots_bitmap);
    list->free_slots_bitmap = new_bitmap;
    list->bitmap_size = new_bitmap_size;
}

static STRU_MEM_LIST* find_or_create_list(size_t size) {
    int target_size = SLOT_ALLINED_SIZE(size);  // align size
    STRU_MEM_LIST* current = mem_pool;
    STRU_MEM_LIST* previous = NULL;
    
    // find a matching or larger slot size
    while (current != NULL) {
        if (current->slot_size == target_size) return current;  // match found
        if (current->slot_size > target_size) break;  // stop at larger size
        previous = current;
        current = current->next_list;
    }
    
    // create a new memory list if no match 
    STRU_MEM_LIST* new_list = create_memory_list(target_size);
    if (!new_list) return NULL;  // Return NULL if allocation fails
    
    // insert new list in sorted order
    if (!previous) {
        new_list->next_list = mem_pool;  // insert at head
        mem_pool = new_list;
    } else {
        new_list->next_list = current;  // insert between nodes
        previous->next_list = new_list;
    }
    
    return new_list;
}

/*
 * Allocate a chunk of memory 	
 * @param size: size in bytes to be allocated
 * @return: the pointer to the allocated memory slot
 */
void* mem_mngr_alloc(size_t size) {
    if (size == 0 || size > 5 * MEM_ALIGNMENT_BOUNDARY) {
        return NULL;
    }

    STRU_MEM_LIST* list = find_or_create_list(size);
    if (!list) return NULL;

    // Find first free slot
    int slot_index = bitmap_find_first_bit(list->free_slots_bitmap, list->bitmap_size, 1);
    
    // If no free slot found, add new batch
    if (slot_index == BITMAP_OP_NOT_FOUND) {
        STRU_MEM_BATCH* curr_batch = list->first_batch;
        while (curr_batch->next_batch) {
            curr_batch = curr_batch->next_batch;
        }
        
        STRU_MEM_BATCH* new_batch = create_memory_batch(list->slot_size);
        if (!new_batch) return NULL;
        
        curr_batch->next_batch = new_batch;
        list->batch_count++;
        
        // Expand bitmap
        expand_bitmap(list);
        
        // Use first slot in new batch
        slot_index = (list->batch_count - 1) * MEM_BATCH_SLOT_COUNT;
    }
    
    // Mark slot as used
    bitmap_clear_bit(list->free_slots_bitmap, list->bitmap_size, slot_index);
    
    // Calculate actual memory address
    int batch_index = slot_index / MEM_BATCH_SLOT_COUNT;
    int slot_in_batch = slot_index % MEM_BATCH_SLOT_COUNT;
    
    STRU_MEM_BATCH* target_batch = list->first_batch;
    for (int i = 0; i < batch_index; i++) {
        target_batch = target_batch->next_batch;
    }
    
    return (char*)target_batch->batch_mem + (slot_in_batch * list->slot_size);
}

/*
 * Free a chunk of memory pointed by ptr
 * Print out any error messages
 * @param: the pointer to the allocated memory slot
 */
void mem_mngr_free(void* ptr) {
    if (!ptr) return;

    STRU_MEM_LIST* curr_list = mem_pool;
    int found = 0;
    
    // Search through all lists
    while (curr_list && !found) {
        STRU_MEM_BATCH* curr_batch = curr_list->first_batch;
        int batch_index = 0;
        
        // Search through all batches in current list
        while (curr_batch && !found) {
            // Check if ptr is within this batch
            if (ptr >= curr_batch->batch_mem && 
                ptr < (void*)((char*)curr_batch->batch_mem + 
                            (MEM_BATCH_SLOT_COUNT * curr_list->slot_size))) {
                
                // Calculate slot index
                ptrdiff_t offset = (char*)ptr - (char*)curr_batch->batch_mem;
                if (offset % curr_list->slot_size != 0) {
                    printf("Error: Invalid pointer - not aligned to slot boundary\n");
                    return;
                }
                
                int slot_in_batch = offset / curr_list->slot_size;
                int global_slot_index = batch_index * MEM_BATCH_SLOT_COUNT + slot_in_batch;
                
                // Check if slot is already free
                if (bitmap_bit_is_set(curr_list->free_slots_bitmap, 
                                    curr_list->bitmap_size, 
                                    global_slot_index)) {
                    printf("Error: Double free detected\n");
                    return;
                }
                
                // Mark slot as free
                bitmap_set_bit(curr_list->free_slots_bitmap, 
                             curr_list->bitmap_size, 
                             global_slot_index);
                found = 1;
            }
            
            curr_batch = curr_batch->next_batch;
            batch_index++;
        }
        
        curr_list = curr_list->next_list;
    }
    
    if (!found) {
        printf("Error: Invalid pointer - not managed by memory manager\n");
    }
}