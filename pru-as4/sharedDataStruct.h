#ifndef _SHARED_DATA_STRUCT_H_
#define _SHARED_DATA_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>
#define STR_LEN 8


// WARNING:
// Fields in the struct must be aligned to match ARM's alignment
//    bool/char, uint8_t:   byte aligned
//    int/long,  uint32_t:  word (4 byte) aligned
//    double,    uint64_t:  dword (8 byte) aligned
// Add padding fields (char _p1) to pad out to alignment.

// My Shared Memory Structure
// ----------------------------------------------------------------
typedef struct {
    // Each color component is represented by a 32-bit unsigned integer (we have 8)
    uint32_t color[STR_LEN];
    //probably good to include some sort of shutdown flag value here and alignment before submission
    //uint32_t padding; // Adjust size as needed

    // Add a shutdown flag if necessary
    // uint32_t shutdown_flag; // Example shutdown flag

} sharedColorStruct_t;

#endif
