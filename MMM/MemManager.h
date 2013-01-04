/*
 *  MemManager.h
 *  MMM
 *
 *  Created by Sahil Ramani on 12/31/12.
 *  Copyright 2012 Upshift Inc. All rights reserved.
 *
 */

#pragma once

#include <cstddef>
#include "EngineConstants.h"

// MemManager : Actual memory manager implementation
//  Singleton class used since we actually need only one instance 
//  to manage total memory
class MemManager 
{
	
public:
	static MemManager* Instance();
	
	void* Allocate(u32 , u8 );
	void SetAllocationStrategy(ALLOCATION_STRATEGY );
	
	void DeAllocate(void * ); 
private:
	// Private constructor and destructor ensure singleton-ness
	MemManager();
	~MemManager();
	
	// The actual singleton instance
	static MemManager* m_pInstance; 
	
	// Memory and tracker units internal to the manager.
	TRACKER_UNIT trackerUnits[NUM_TRACKER_UNITS];
	u8 *m_pMemory;
	
	// Internal methods
	s16 FindUsableTrackingUnitID(const u32&, const u8& , TRACKER_UNIT&);
	MEMORY_ADDRESS* GetUsableMemoryAddressFromTrackerID(const u16& , const TRACKER_UNIT&);
	void MarkMemoryAddressUsed(const u16& , const TRACKER_UNIT& , const u32& , const TRACKER_UNIT& );

	// Internal values
	ALLOCATION_STRATEGY m_AllocStrategy;
};

