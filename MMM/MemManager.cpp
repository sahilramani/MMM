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
	memset((void*)trackerUnits, 0, sizeof(TRACKER_UNIT) * NUM_TRACKER_UNITS );
	
	// Set default allocation strategy = FIRST_FIT;
	m_AllocStrategy = FIRST_FIT;
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
		
	u16 trackerID = FindUsableTrackingUnitID(size, alignment);
	
	// Set memory as used and return the address
	MEMORY_ADDRESS tpMemory = GetUsableMemoryAddressFromTrackerID(trackerID);
	
	// Fill out allocation header
	memset((void*)((MEMORY_ADDRESS)tpMemory), 0, headerPaddingSize);
	((ALLOCATION_HEADER*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize))->size			= size;
	((ALLOCATION_HEADER*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize))->alignment		= alignment;
	
	// Return the actual memory location
	return (void*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize);
}

// Takes the size / alignment and finds the best location for the memory
//   using only trackers. No memory involved yet. 
u16 MemManager::FindUsableTrackingUnitID(const u32& size, const u8& alignment )
{
	// Default fail tracker unit
	u8 trackerID = -1;

	// Calculate the required number of pages
	u32 numRequiredPages = size / CHUNK_SIZE + (size % CHUNK_SIZE) > 0 ? 1 : 0;
	
	// .. and then, total tracker units required.
	u32 numRequiredTrackerUnits = numRequiredPages / TRACKER_UNIT_SIZE + 
								( numRequiredPages % TRACKER_UNIT_SIZE ) > 0 ? 1 : 0;
	
	// ... fail out if requested memory is more than the total memory.
	if(numRequiredTrackerUnits <= NUM_TRACKER_UNITS)
	{
		// If not, let's build a bitmask.
		TRACKER_UNIT partialBitMask = (TRACKER_UNIT)(TRACKING_UNIT_ALL_USED << (size % CHUNK_SIZE));

		// To keep track of the current tracking unit being checked
		u16 current_tracking_unit=0;
		
		// Lets get into the meat of things now.. given an allocation strategy, find a tracker unit
		switch (m_AllocStrategy) {
			case FIRST_FIT:
				
			while(true)
			{
				// Find first tracker unit with empty slots
				while (trackerUnits[current_tracking_unit] & TRACKING_UNIT_ALL_USED == TRACKING_UNIT_ALL_USED) {
					current_tracking_unit++;
				}
				
				// Fail out if we can't find even one. This probably means we've exhausted memory
				if( current_tracking_unit >= NUM_TRACKER_UNITS)
					break;
				else {
					u16 j=1;
					// From the tracking unit we found, check if the next (numRequiredTrackerUnits) are empty
					while(j<numRequiredTrackerUnits)
					{
						if(((current_tracking_unit + j) < NUM_TRACKER_UNITS) && 
						   ((~trackerUnits[current_tracking_unit + j]) & TRACKING_UNIT_ALL_USED != TRACKING_UNIT_ALL_USED))
							break;
						j++;
					}
					
					// Oh no, we couldn't find enough memory space, let's loop back
					if (j < numRequiredTrackerUnits) {
						current_tracking_unit += j;
						continue;
					}
					// We found enough empty tracker units, now we need to see if the partialbitmask can be satisfied
					else {
						// Now we need to check for the partialBitMask
						if(~trackerUnits[current_tracking_unit] & partialBitMask == partialBitMask )
						{
							// We can fit the partial bitmask into the first tracker unit, 
							// so we're basically done here.
							trackerID = current_tracking_unit;
							break;
						}
						else {
							u32 shift_count = 1;
							// Calculate how many pages can fit in the first tracker unit
							while((partialBitMask<<shift_count) && 
								  (~trackerUnits[current_tracking_unit] & (partialBitMask<<shift_count) != partialBitMask))
							{
								++shift_count;
							}
							
							// Calculate the bitmask for the remaining pages needed.
							TRACKER_UNIT secondPartialBitmask = TRACKING_UNIT_ALL_USED << (NUM_PAGES_PER_UNIT-shift_count);
							
							// Check to see if there's another page available
							if(((current_tracking_unit+j+1) < NUM_TRACKER_UNITS) && 
							   (~trackerUnits[current_tracking_unit+j+1] & secondPartialBitmask != secondPartialBitmask))
							{
								trackerID = current_tracking_unit;
								++numRequiredTrackerUnits;
								break;
							}
							else
							   continue;
						}
					}
				}
			}
			break;
						
			default:
				break;
		}
	}
	return (u8)trackerID;
}

// Returns the final memory address given a tracker id.
MEMORY_ADDRESS MemManager::GetUsableMemoryAddressFromTrackerID(const u16& trackerID)
{
	return (MEMORY_ADDRESS)m_pMemory;
}