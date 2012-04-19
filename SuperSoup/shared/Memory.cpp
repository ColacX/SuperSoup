#include "Memory.h"

#undef new

MemoryDebug::Memory& MemoryDebug::operator()(const char* file, const char* func, int line){
	if(isEnabled){
		isEnabled = false;
		Memory* m = new Memory();
		isEnabled = true;

		// Skip the filepath to the file when in debug-mode
		#ifdef _DEBUG
			const char* tmp = strrchr(file,'\\');
			m->file = tmp==NULL ? file : tmp + 1;
		#else
			m->file = file;
		#endif

		m->func = func;
		m->line = line;
		m->isArray = isArray;

		return *m;
	}
	else
		return *(Memory*)NULL;
}


void MemoryDebug::debugPrint(){
	printf("MemoryLeaks[%u] {\n",map.size());
	for(auto it = map.begin(); it!=map.end(); ++it){
		printf("  0x%X = %s",it->second->ptr, it->second->type);
		if(it->second->isArray)
			printf("[]");
		printf(" @%s:%s[%d]\n",it->second->file, it->second->func, it->second->line);
	}
	printf("}\n");
}

std::map< void*, MemoryDebug::Memory* > MemoryDebug::map;
bool MemoryDebug::isEnabled = false;
bool MemoryDebug::isArray;

MemoryDebug memoryDebug;

void * __cdecl operator new(size_t size){
	void *memory = malloc( size );
	if(MemoryDebug::isEnabled)
		MemoryDebug::isArray = false;
	return memory;
}

void * __cdecl operator new[]( size_t size ){
	void *memory = malloc( size );
	if(MemoryDebug::isEnabled)
		MemoryDebug::isArray = true;
	return memory;
}

void __cdecl operator delete( void* memory ){
	if(MemoryDebug::isEnabled)
		memoryDebug.del(memory,false);
	free( memory );
}
void __cdecl operator delete[]( void* memory ){
	if(MemoryDebug::isEnabled)
		memoryDebug.del(memory,true);
	free( memory );
}
