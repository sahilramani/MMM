/*
 *  MemManager.cpp
 *  MMM
 *
 *  Created by Sahil Ramani on 12/31/12.
 *  Copyright 2012 Upshift Inc. All rights reserved.
 *
 */

#ifndef _MEMMANAGER_H_
#define _MEMMANAGER_H_
#endif

#include "MemManager.h"
#include <cstring>

MemManager* MemManager::m_pInstance = NULL;

MemManager::MemManager()
{
	
	// This is the memory to be tracked. 
	// TODO : Find a generic way to "place" memory. 
	//        Basically find memory that can be used based on platform.
	//  * The use of new here is temporary and should not be considered final *
	m_pMemory = new u8[MEMORY_SIZE];
	
	// Initialize tracker units, initially none used
	memset((void*)TRACKER_UNITS, 0, sizeof(TRACKER_UNIT) * NUM_TRACKER_UNITS );
	
}

MemManager::~MemManager()
{
	
	// This is the memory to be tracked. 
	// TODO : Find a generic way to "place" memory. 
	//        Basically find memory that can be used based on platform.
	//  * The use of new here is temporary and should not be considered final *
	if(m_pMemory) delete [] m_pMemory;
}

MemManager* MemManager::Instance()
{
	if(!m_pInstance)
		m_pInstance = new MemManager();
	
	return m_pInstance;
}

void* MemManager::Allocate(u32 size, u8 alignment)
{
	// Here, we need to do some calculations. 
	//
	//  Lets first pad the size to the requested alignment 
	size += ((size % alignment) > 0) ? alignment - (size % alignment) : 0;
	
	// Now lets calculate the header padding required for alignment
	u32 headerPaddingSize = ((ALLOCATION_HEADER_SIZE % alignment ) > 0 ? 
							 alignment - (ALLOCATION_HEADER_SIZE % alignment) :
							 0);
	
	// And then let's total up the total header size with padding
	u32 headerSize = ALLOCATION_HEADER_SIZE + headerPaddingSize;
	
	// And finally, the total size = header size + memory size
	size += headerSize;
	
	
}