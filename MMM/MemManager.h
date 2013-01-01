/*
 *  MemManager.h
 *  MMM
 *
 *  Created by Sahil Ramani on 12/31/12.
 *  Copyright 2012 Upshift Inc. All rights reserved.
 *
 */

#include <cstddef>

#pragma once

// Lets define some constants for engine use

#define _MB(size) ((size)*1024*1024)

#define u8  char
#define u16 unsigned 
#define u32 unsigned long 
#define u64 unsigned long long 

#define TRACKER_UNIT  u32 


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
const u32 NUM_PAGES_PER_UNIT = 8*sizeof(TRACKER_UNIT);

// Tracker unit size : Refers to the actual tracker to manage memory
//  4 bytes per TRACKER_UNIT means we'll be tracking upto 32 
//   different memory chunks per tracker unit.
//  Also, that's about (32 * CHUNK_SIZE) bytes of data per tracker
const u32 NUM_TRACKER_UNITS = ((MEMORY_SIZE)/((CHUNK_SIZE)*NUM_PAGES_PER_UNIT));


// MemManager : Actual memory manager implementation
//  Singleton class used since we actually need only one instance 
//  to manage total memory
class MemManager 
{
	
public:
	static MemManager* Instance();
	
private:
	// Private constructor and destructor ensure singleton-ness
	MemManager();
	~MemManager();
	
	// The actual singleton instance
	static MemManager* m_pInstance; 
	
	// Memory and tracker units internal to the manager.
	TRACKER_UNIT TRACKER_UNITS[NUM_TRACKER_UNITS];
	u8 *m_pMemory;
};

