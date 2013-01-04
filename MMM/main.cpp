#include "globals.h"

int main (int argc, char * const argv[]) {
    // insert code here...
    std::cout << "Nothing here yet! Hold on for implementation completion first.\n";
    
	void *ptr = MemManager::Instance()->Allocate(1024, 4);
	MemManager::Instance()->DeAllocate(ptr);
	return 0;
	
	
}
