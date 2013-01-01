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

MemManager* MemManager::m_pInstance = NULL;

MemManager::MemManager()
{
	
	// This is the memory to be tracked. 
	// TODO : Find a generic way to "place" memory. 
	//        Basically find memory that can be used based on platform.
	//  * The use of new here is temporary and should not be considered final *
	m_pMemory = new u8[MEMORY_SIZE];
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