#include "globals.h"

int main (int argc, char * const argv[]) {
    // insert code here...
    std::cout << "Nothing here yet! Hold on for implementation completion first.\n";

	s32 num_elements = 1000;
	s32 num_alloc = 0;
	s32 num_failed = 0;
	void *ptr[1000];
    
	for(int i=0;i<num_elements;i++)
	{
		ptr[i] = MemManager::Instance()->Allocate((1024 * 32 - 4), 4);
		if(!(ptr[i]))	num_failed++; 
		else			num_alloc++;

	}
	for(int i=0; i<1000;i++)
	{ 
		if(ptr[i])
			MemManager::Instance()->DeAllocate(ptr[i]);
	}
	
	std::cout<< "Num Alloc : "<<num_alloc<<"\nNum Failed : "<<num_failed<<"\n";
	return 0;
}
