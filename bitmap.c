/*
 * CS 551 Project "Memory manager".
 * This file needs to be turned in.	
 */

#include "common.h"

int bitmap_print_bitmap(unsigned char* bitmap, int size) {
    if (!bitmap || size <= 0) {
        return BITMAP_OP_ERROR;
    }

    printf("\tBitmap content: ");
    for (int i = 0; i < size; i++) {
        // Print each bit of the current byte
        for (int j = 0; j < BIT_PER_BYTE; j++) {
            printf("%d", (bitmap[i] >> j) & 0x01);
        }
        printf(" ");
    }
    printf("\n");

    return BITMAP_OP_SUCCEED;
}

int bitmap_find_first_bit(unsigned char * bitmap, int size, int val) {
    if (!bitmap || size <= 0) {
        return BITMAP_OP_ERROR;
    }

    int total_bits = size * BIT_PER_BYTE;
    
    for (int pos = 0; pos < total_bits; pos++) {
        // Get current byte and bit position within byte
        unsigned char byte = bitmap[pos / BIT_PER_BYTE];
        int bit_pos = pos % BIT_PER_BYTE;
        
        // Extract bit value using right shift and mask
        int bit_val = (byte >> bit_pos) & 0x01;
        
        // Return position if we found matching bit value
        if ((val && bit_val) || (!val && !bit_val)) {
            return pos;
        }
    }
    
    return BITMAP_OP_NOT_FOUND;
}

int bitmap_set_bit(unsigned char * bitmap, int size, int target_pos) {
    if (!bitmap || size <= 0 || target_pos < 0) {
        return BITMAP_OP_ERROR;
    }

    int total_bits = size * BIT_PER_BYTE;
    if (target_pos >= total_bits) {
        return BITMAP_OP_ERROR;
    }

    // Calculate byte offset and bit position
    int byte_offset = target_pos / BIT_PER_BYTE;
    int bit_pos = target_pos % BIT_PER_BYTE;
    
    // Set bit using OR with shifted 1
    bitmap[byte_offset] |= (1 << bit_pos);
    
    return BITMAP_OP_SUCCEED;
}

int bitmap_clear_bit(unsigned char * bitmap, int size, int target_pos) {
    if (!bitmap || size <= 0 || target_pos < 0) {
        return BITMAP_OP_ERROR;
    }

    int total_bits = size * BIT_PER_BYTE;
    if (target_pos >= total_bits) {
        return BITMAP_OP_ERROR;
    }

    // Calculate byte offset and bit position
    int byte_offset = target_pos / BIT_PER_BYTE;
    int bit_pos = target_pos % BIT_PER_BYTE;
    
    // Clear bit using AND with inverted mask
    bitmap[byte_offset] &= ~(1 << bit_pos);
    
    return BITMAP_OP_SUCCEED;
}

int bitmap_bit_is_set(unsigned char * bitmap, int size, int pos) {
    if (!bitmap || size <= 0 || pos < 0) {
        return BITMAP_OP_ERROR;
    }

    int total_bits = size * BIT_PER_BYTE;
    if (pos >= total_bits) {
        return BITMAP_OP_ERROR;
    }

    // Calculate byte offset and bit position
    int byte_offset = pos / BIT_PER_BYTE;
    int bit_pos = pos % BIT_PER_BYTE;
    
    // Extract and return bit value
    return (bitmap[byte_offset] >> bit_pos) & 0x01;
}