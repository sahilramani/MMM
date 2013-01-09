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
		
	TRACKER_UNIT startFinalBitMask = (TRACKER_UNIT)0x0;
	s16 trackerID = FindUsableTrackingUnitID(size, alignment, startFinalBitMask);
	if(trackerID == -1) return NULL;
	
	// Set memory as used and return the address
	MEMORY_ADDRESS* tpMemory = GetUsableMemoryAddressFromTrackerID(trackerID, startFinalBitMask);
	
	// Fill out allocation header
	memset((void*)(tpMemory), 0, headerPaddingSize);
	((ALLOCATION_HEADER*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize))->size			= size;
	((ALLOCATION_HEADER*)(tpMemory + (MEMORY_ADDRESS)headerPaddingSize))->alignment		= alignment;
	
	// Return the actual memory location
	return (void*)(tpMemory + (MEMORY_ADDRESS)headerSize);
}

// Takes the size / alignment and finds the best location for the memory
//   using only trackers. No memory involved yet. 
s16 MemManager::FindUsableTrackingUnitID(const u32& size, const u8& alignment , TRACKER_UNIT& startFinalBitMask)
{
	// Default fail tracker unit
	s16 trackerID = -1;

	// Calculate the required number of pages
	u32 numRequiredPages = size / CHUNK_SIZE + ((size % CHUNK_SIZE) > 0 ? 1 : 0);
	
	// .. and then, total tracker units required.
	u32 numRequiredTrackerUnits = numRequiredPages / NUM_PAGES_PER_UNIT + 
								(( numRequiredPages % NUM_PAGES_PER_UNIT ) > 0 ? 1 : 0);
	
	// To keep track of the current tracking unit being checked
	u16 current_tracking_unit = 0;
	TRACKER_UNIT last_unit_bitmask = TRACKING_UNIT_ALL_USED;
	
	// ... fail out if requested memory is more than the total memory.
	if(numRequiredTrackerUnits <= NUM_TRACKER_UNITS)
	{
		// If not, let's build a bitmask.ges
		TRACKER_UNIT partialBitMask = (TRACKER_UNIT)((TRACKING_UNIT_ALL_USED << (NUM_PAGES_PER_UNIT - (numRequiredPages % NUM_PAGES_PER_UNIT))));

		// Lets get into the meat of things now.. given an allocation strategy, find a tracker unit
		switch (m_AllocStrategy) {
			case FIRST_FIT:
				
			while(true)
			{
				// Find first tracker unit with empty slots
				while (trackerUnits[current_tracking_unit] == TRACKING_UNIT_ALL_USED) {
					current_tracking_unit++;
				}
				
				// Fail out if we can't find enough trackers. This probably means we've exhausted memory
				// Negative one ensures the current tracker is also considered
				if( (current_tracking_unit + numRequiredTrackerUnits -1) >= NUM_TRACKER_UNITS)
					break;
				else {
					
					if(numRequiredTrackerUnits == 1)
					{
						// We only need one tracking unit for the requested memory (or so we think, for now)
						if (((~trackerUnits[current_tracking_unit]) & partialBitMask) == partialBitMask) {
							// The current unit has enough spaces for the entire memory, so we have a winner. 
							// Now, let's adjust the memory to fit appropriately
							
							if (!trackerUnits[current_tracking_unit]) {
								// Tracker unit empty, let's just fit at the start, this is easy
								partialBitMask = ~(TRACKING_UNIT_ALL_USED << numRequiredPages);
							}
							else {
								// This is tricky. The tracking unit isn't empty, so shift right 
								//  till we can find the appropriate location 
								do
								{
									partialBitMask >>= 1;
								}while ((partialBitMask & ~trackerUnits[current_tracking_unit]) == partialBitMask);
								
								// Oops, we crossed one, let's shift back.
								partialBitMask <<= 1;
							}
							trackerID = current_tracking_unit;
							startFinalBitMask = partialBitMask;
							break;
						}
						else {
							// The tracker unit doesn't have enough to fill the requested memory, but has 
							//  enough to fill some.
							++numRequiredTrackerUnits;

							// Let's just safely do the first shift, so we can continue...
							last_unit_bitmask = 1;
							partialBitMask <<=1;
							while ((partialBitMask & ~trackerUnits[current_tracking_unit]) != partialBitMask) {
								partialBitMask <<= 1;
								last_unit_bitmask |= (last_unit_bitmask<<1) ;
							}
							if (~trackerUnits[current_tracking_unit + numRequiredTrackerUnits - 1] & last_unit_bitmask != last_unit_bitmask) {
								// The next tracker unit doesn't have enough space. Fail out;
								continue;
							}
							trackerID = current_tracking_unit;
							startFinalBitMask = partialBitMask;
							break;
						}

					}
					else {
						// We need more than one tracking unit to fit the requested memory
						last_unit_bitmask = TRACKING_UNIT_ALL_USED;
						
						if (~trackerUnits[current_tracking_unit] & partialBitMask != partialBitMask) {
							// The first tracker cannot the partial memory requested
							// This means there's fewer slots. 
							// Lets calculate the bitmask for the additional slot required for this allocation.
							++numRequiredTrackerUnits;
							
							// Check for overflow
							if(current_tracking_unit + numRequiredTrackerUnits > NUM_TRACKER_UNITS ) return NULL;
							
							// Let's just safely do the first shift, so we can continue...
							last_unit_bitmask = 1;
							partialBitMask <<=1;
							while (partialBitMask & ~trackerUnits[current_tracking_unit] != partialBitMask) {
								partialBitMask <<= 1;
								last_unit_bitmask |= (last_unit_bitmask<<1) ;
							}
						}
						else {
							TRACKER_UNIT tempBitMask = partialBitMask;
							partialBitMask = ~trackerUnits[current_tracking_unit];
							// If first tracker is empty
							if (!trackerUnits[current_tracking_unit]) {
								// While the first set bit is not the last
								while(!(tempBitMask & 1))
								{
									tempBitMask >>=1;
								}
							}
							else {
								// While we get zeros for ANDing both tempBitMask and partialBitMask.. we keep shifting
								// Basically,	1000 & 0001 = 0
								//				0100 & 0001 = 0
								//				0010 & 0001 = 0
								//				0001 & 0001 = 1; viola!
								while (!(tempBitMask & trackerUnits[current_tracking_unit])) {
									tempBitMask >>= 1;
								}
								// Because this is the last value, we need to shift back to find the last empty slot
								tempBitMask <<= 1;
							}

							last_unit_bitmask  = tempBitMask | trackerUnits[current_tracking_unit];
							
						}
						u32 j=0;
						for (; j<numRequiredTrackerUnits-1; j++) {
							if (~trackerUnits[current_tracking_unit + j] & TRACKING_UNIT_ALL_USED != TRACKING_UNIT_ALL_USED) {
								break;
							}
						}
						
						// We couldn't find enough units to satisfy n-1 tracker unit's worth pages
						if ((j!= numRequiredTrackerUnits-1) || 
							(~trackerUnits[current_tracking_unit + numRequiredTrackerUnits - 1] 
											& TRACKING_UNIT_ALL_USED != TRACKING_UNIT_ALL_USED)) 
						{
							continue;
						}
						
						// If not, we have a winner...
						trackerID = current_tracking_unit;
						startFinalBitMask = partialBitMask;

						
						break;
					}

				}
			}
			break;
						
			default:
				break;
		}
	}
	
	
	// Lets mark units as used if we found something.
	if (trackerID != -1) {
		MarkMemoryAddressUsed(current_tracking_unit, startFinalBitMask, numRequiredTrackerUnits, last_unit_bitmask);
	}
	
	return (s16)trackerID;
}

void MemManager::MarkMemoryAddressUsed(const u16& trackerID, const TRACKER_UNIT& startBitMask, const u32& numTrackerUnits, const TRACKER_UNIT& finalBitMask)
{
	trackerUnits[trackerID] |= startBitMask;
	for (u32 j=1; j<numTrackerUnits-1; j++) {
		trackerUnits[trackerID + j] |= TRACKING_UNIT_ALL_USED;
	}
	if(numTrackerUnits > 1)
		trackerUnits[trackerID+numTrackerUnits-1] |= finalBitMask;
}

// Returns the final memory address given a tracker id.
MEMORY_ADDRESS* MemManager::GetUsableMemoryAddressFromTrackerID(const u16& trackerID, const TRACKER_UNIT& startFinalBitMask)
{
	u32 shift_count = 0;
	if (~startFinalBitMask) {
		while (!(startFinalBitMask& (0x1<<shift_count))) {
			++shift_count;
		}
	}
	return (MEMORY_ADDRESS*)(m_pMemory + trackerID * NUM_BYTES_PER_UNIT + shift_count * CHUNK_SIZE);
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
	u32 numRequiredPages = size / CHUNK_SIZE + ((size % CHUNK_SIZE)>0 ? 1 : 0);

	// We must understand that pointer differences tell us the number of values stored in the particular object 
	//  size. Hence multiplying it with the size gives us the true byte difference.
	MEMORY_ADDRESS memoryOffset = ((MEMORY_ADDRESS*)pMemory - (MEMORY_ADDRESS*)m_pMemory ) * sizeof(MEMORY_ADDRESS);
	TRACKER_UNIT partialBitMask = (TRACKER_UNIT)(TRACKING_UNIT_ALL_USED << (NUM_PAGES_PER_UNIT - numRequiredPages % NUM_PAGES_PER_UNIT)); 
	TRACKER_UNIT memoryBitMask  = (TRACKER_UNIT)(TRACKING_UNIT_ALL_USED << 
												 ((memoryOffset % NUM_BYTES_PER_UNIT) / CHUNK_SIZE));
	
	u32 startTrackerID = memoryOffset / NUM_BYTES_PER_UNIT;
	
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
		else {
			// Adjustment was to the right, that means there were more units remaining
			//  so there was a reduction in the last 
			u8 last_unshift_count=0;
			do {
				++last_unshift_count;
				partialBitMask = partialBitMask | (partialBitMask >> 1);
			} while (~trackerUnits[startTrackerID] != partialBitMask && 
					 partialBitMask != TRACKING_UNIT_ALL_USED) ;
			
			if(partialBitMask == TRACKING_UNIT_ALL_USED)
			{
				//cout<<" THis is an error.";
				//break;
			}
			
			// Mark the first tracker unit as unused
			trackerUnits[startTrackerID] &= ~partialBitMask;
			
			// Mark all but the last tracker unit as unused
			for(int j=1; j<numRequiredTrackerUnits-1; j++)
				trackerUnits[startTrackerID + j] &= ~(TRACKING_UNIT_ALL_USED);
			
			// For the last unit, mark using lastUnitBitMask
			trackerUnits[numRequiredTrackerUnits-1] &= ~(TRACKING_UNIT_ALL_USED >> last_unshift_count);
		}

	}
	else {
		trackerUnits[startTrackerID] &= partialBitMask;
		for(int j=1; j<numRequiredTrackerUnits; j++)
			trackerUnits[startTrackerID + j] &= ~(TRACKING_UNIT_ALL_USED);
	}

}
