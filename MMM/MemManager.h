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
	
	void* Allocate(u32 size, u8 alignment);
	
private:
	// Private constructor and destructor ensure singleton-ness
	MemManager();
	~MemManager();
	
	// The actual singleton instance
	static MemManager* m_pInstance; 
	
	// Memory and tracker units internal to the manager.
	TRACKER_UNIT TRACKER_UNITS[NUM_TRACKER_UNITS];
	u8 *m_pMemory;
	
	// Internal methods
	u8 FindUsableTrackingUnitID(const u32&, const u8& );
	MEMORY_ADDRESS GetUsableMemoryAddressFromTrackerID(u16 );
};

