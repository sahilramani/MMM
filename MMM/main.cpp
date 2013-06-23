#include "globals.h"
#define NUM_ELEMENTS 2000

int main (int argc, char * const argv[]) {
    // insert code here...
    std::cout << "Nothing here yet! Hold on for implementation completion first.\n";

	s32 num_elements = NUM_ELEMENTS;
	s32 num_alloc = 0;
	s32 num_failed = 0;
	void *ptr[NUM_ELEMENTS];
    
	for(int i=0;i<num_elements;i++) {
		ptr[i] = MemManager::Instance()->Allocate((int)(rand() * 31), 4);
		if(!(ptr[i])) {
			num_failed++;
		} else {
			num_alloc++;
		}
		if((int)(rand()) %2 == 0)
				MemManager::Instance()->DeAllocate(ptr[(int)((float)rand()/(float)RAND_MAX) * i]);
	}
	for(int i=0; i<1000;i++) { 
		if(ptr[i]) {
			MemManager::Instance()->DeAllocate(ptr[i]);
		}
	}
	
	std::cout<< "Num Alloc : "<<num_alloc<<"\nNum Failed : "<<num_failed<<"\n";
	return 0;
}
