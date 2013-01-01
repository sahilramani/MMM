/*
 *  EngineConstants.h
 *  MMM
 *
 *  Created by Sahil Ramani on 1/1/13.
 *  Copyright 2013 Upshift Inc. All rights reserved.
 *
 */

// Lets define some constants for engine use

#define _MB(size) ((size)*1024*1024)

#define u8  char
#define u16 unsigned 
#define u32 unsigned long 
#define u64 unsigned long long 

#define TRACKER_UNIT  u32 

// Currently supports 32-bit addresses
// TODO: Add support for 64-bit addresses
#define MEMORY_ADDRESS u32


// Memory to be tracked 
//  Total amount of memory to be used in the system.
//   Keeping at 32 MB since that's what we need to store the 
//   total count of bytes
const u32 MEMORY_SIZE = _MB(32);

// Memory chunk size
//  The size of each memory chunk used
//  Lets start with 1024 bytes, then scale up or down to see performance
const u16  CHUNK_SIZE = 1024;

// Lets start building some useful values for use with our implementation
const u16 NUM_PAGES_PER_UNIT = 8*sizeof(TRACKER_UNIT);

// Tracker unit size : Refers to the actual tracker to manage memory
//  4 bytes per TRACKER_UNIT means we'll be tracking upto 32 
//   different memory chunks per tracker unit.
//  Also, that's about (32 * CHUNK_SIZE) bytes of data per tracker
const u32 NUM_TRACKER_UNITS = ((MEMORY_SIZE)/((CHUNK_SIZE)*NUM_PAGES_PER_UNIT));

// Allocation header : This sets up the header for memory to be used.
//   Every memory block allocated will start with a header. 
//   This gives us enough information regarding the allocation
typedef struct _ALLOCATION_HEADER{
	u32 size;
	u8  alignment;
}ALLOCATION_HEADER;
const u16 ALLOCATION_HEADER_SIZE = sizeof(ALLOCATION_HEADER);
