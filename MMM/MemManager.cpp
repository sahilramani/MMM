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
		
	TRACKER_UNIT startFinalBitMask = (TRACKER_UNIT)0x0f;
	u16 trackerID = FindUsableTrackingUnitID(size, alignment, startFinalBitMask);
	
	// Set memory as used and return the address
	MEMORY_ADDRESS tpMemory = GetUsableMemoryAddressFromTrackerID(trackerID, size, startFinalBitMask);
	
	// Fill out allocation header
	memset((void*)((MEMORY_ADDRESS)tpMemory), 0, headerPaddingSize);
	((ALLOCATION_HEADER*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize))->size			= size;
	((ALLOCATION_HEADER*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize))->alignment		= alignment;
	
	// Return the actual memory location
	return (void*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize);
}

// Takes the size / alignment and finds the best location for the memory
//   using only trackers. No memory involved yet. 
u16 MemManager::FindUsableTrackingUnitID(const u32& size, const u8& alignment , TRACKER_UNIT& startFinalBitMask)
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
							TRACKER_UNIT shift_count = 1;
							// Calculate how many pages can fit in the first tracker unit
							while((partialBitMask<<shift_count) && 
								  (~trackerUnits[current_tracking_unit] & (partialBitMask<<shift_count) != partialBitMask))
							{
								++shift_count;
							}
							
							// Calculate the bitmask for the remaining pages needed.
							TRACKER_UNIT secondPartialBitmask = TRACKING_UNIT_ALL_USED << (NUM_PAGES_PER_UNIT-shift_count);
							
							// Check to see if there's another page available and if it can store the remaining pages
							if(((current_tracking_unit+j+1) < NUM_TRACKER_UNITS) && 
							   (~trackerUnits[current_tracking_unit+j+1] & secondPartialBitmask != secondPartialBitmask))
							{
								trackerID = current_tracking_unit;
								// We need one more tracking unit
								// TODO : This isn't required actually, just for documentative clarity.
								++numRequiredTrackerUnits;
								break;
							}
							// If not, loop back. Rolling back should handle the all the pages used condition automatically
							else
							{
								current_tracking_unit += j;
								continue;
							}
						}
					}
				}
			}
			break;
						
			default:
				break;
		}
	}
	
	// Lets mark this as used for now, we'll remove it 
	return (u8)trackerID;
}

// Returns the final memory address given a tracker id.
MEMORY_ADDRESS MemManager::GetUsableMemoryAddressFromTrackerID(const u16& trackerID, const u32& size, const TRACKER_UNIT& startFinalBitMask)
{
	// Find the first usable memory page in the given tracker
	TRACKER_UNIT shift_count = 0;
	while ((startFinalBitMask>>shift_count) & 0x1f) {
		++shift_count;
	}
	
	// Mark used pages in tracker
	// .. and then, total tracker units required.
	u32 numRequiredTrackerUnits = size / NUM_BYTES_PER_UNIT + 
		( size % NUM_BYTES_PER_UNIT ) > 0 ? 1 : 0;
	
	TRACKER_UNIT partialBitMask = (TRACKER_UNIT)(TRACKING_UNIT_ALL_USED << (size % CHUNK_SIZE));
	
	if(partialBitMask != startFinalBitMask) 
	{
		// Temporary:
		u32 j=0;
		
		++numRequiredTrackerUnits;
		
		// Mark first tracker unit
		trackerUnits[trackerID] |= startFinalBitMask;
		
		// Mark all but the last tracker unit
		for (j=1; j<numRequiredTrackerUnits-1; j++) {
			trackerUnits[trackerID+j] |= TRACKING_UNIT_ALL_USED;
		}
		
		// Calculate how many bits need to be marked for the last unit.
		TRACKER_UNIT last_shift_mask = 0;
		j=shift_count;
		while (((partialBitMask>>j)&(startFinalBitMask>>j)) != (partialBitMask>>j)) {
			++j;
			last_shift_mask<<=1;
			++last_shift_mask;
		}
		
		// Mark the last unit
		trackerUnits[trackerID+numRequiredTrackerUnits-1] |= last_shift_mask;
	}
	else {
		// Mark first tracker unit
		trackerUnits[trackerID] |= startFinalBitMask;
		
		// Mark all but the last tracker unit
		for (u32 j=1; j<numRequiredTrackerUnits; j++) {
			trackerUnits[trackerID+j] |= TRACKING_UNIT_ALL_USED;
		}
		
	}

	return (MEMORY_ADDRESS)(m_pMemory + trackerID * NUM_BYTES_PER_UNIT + shift_count * CHUNK_SIZE);
}

// Let's also free this memory now
void MemManager::DeAllocate(void *pMemory)
{
	// The header is just before the memory address in the scheme of things
	u32 size = ((ALLOCATION_HEADER*)((MEMORY_ADDRESS*)pMemory-ALLOCATION_HEADER_SIZE))->size;
	u32 alignment = ((ALLOCATION_HEADER*)((MEMORY_ADDRESS*)pMemory-ALLOCATION_HEADER_SIZE))->alignment;
	
	// Now, to calculate the true starting point of used memory, we need to calculate
	//  the padding used for the header
	u32 headerPaddingSize = (ALLOCATION_HEADER_SIZE % alignment > 0) ? alignment - (ALLOCATION_HEADER_SIZE % alignment) : 0;
	
	// Calculate the true start memory location
	pMemory = (MEMORY_ADDRESS*)pMemory - (MEMORY_ADDRESS)ALLOCATION_HEADER_SIZE - (MEMORY_ADDRESS)headerPaddingSize;
	
	// Calculate and store the partialBitMask and the number of tracker units used
	u32 numRequiredTrackerUnits = (size / NUM_BYTES_PER_UNIT) + ((size % NUM_BYTES_PER_UNIT) > 0 ? 1 : 0);
	
	TRACKER_UNIT partialBitMask = (TRACKER_UNIT)(TRACKING_UNIT_ALL_USED << (size % CHUNK_SIZE)); 
	TRACKER_UNIT memoryBitMask  = (TRACKER_UNIT)(TRACKING_UNIT_ALL_USED << 
												 ((MEMORY_ADDRESS*)pMemory - (MEMORY_ADDRESS*)m_pMemory) % CHUNK_SIZE);
	
	u32 startTrackerID = ((MEMORY_ADDRESS*)pMemory - (MEMORY_ADDRESS*)m_pMemory) / CHUNK_SIZE;
	
	if(partialBitMask != memoryBitMask)
	{
		// There was adjustment, handle that
		if(partialBitMask > memoryBitMask)
		{
			// Adjustment was to the left, that means there were fewer units remaining, 
			//  so there was one unit added at the end
			++numRequiredTrackerUnits;
			TRACKER_UNIT lastUnitBitMask = (TRACKER_UNIT)0x0f;
			while (partialBitMask && partialBitMask != memoryBitMask) {
				lastUnitBitMask <<= 1;
				++lastUnitBitMask;
				partialBitMask <<= 1;
			}
			
			if(partialBitMask)
			{
				//printf("Something bad happened, bitmask shouldn't be empty");
				return;
			}
			
			// Mark the first tracker unit as unused
			trackerUnits[startTrackerID] &= ~partialBitMask;
			
			// Mark all but the last tracker unit as unused
			for(int j=1; j<numRequiredTrackerUnits-1; j++)
				trackerUnits[startTrackerID + j] &= ~(TRACKING_UNIT_ALL_USED);
			
			// For the last unit, mark using lastUnitBitMask
			trackerUnits[numRequiredTrackerUnits-1] &= ~lastUnitBitMask;
		}
	}
	else {
		trackerUnits[startTrackerID] &= ~partialBitMask;
		for(int j=1; j<numRequiredTrackerUnits; j++)
			trackerUnits[startTrackerID + j] &= ~(TRACKING_UNIT_ALL_USED);
	}

}