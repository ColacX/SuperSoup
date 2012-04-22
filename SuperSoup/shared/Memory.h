#pragma once
#include <map>
#include <cassert>

class MemoryDebug{
public:
	static bool isEnabled;
	static bool isArray;
	
	class Memory{
	public:
		void* ptr;
		const char* file;
		const char* func;
		int line;
		const char* type;

		bool isArray;

		template<typename T>
		T* operator=(T* t){
			if(isEnabled && this){
				ptr = (void*)t;
				type = typeid(T).name();
				
				isEnabled = false;
				MemoryDebug::map[ptr] = this;
				isEnabled = true;
			}
			return t;
		}
		/*
		template<typename T>
		operator T*&(){
			return *(T*)ptr;
		}*/
		template<typename T>
		operator T&(){
			return *(T*)&ptr;
		}
	};

	static std::map< void*, Memory* > map;

	Memory& operator()(const char* file, const char* func, int line);

	
	template<typename T>
	void* del(T* memory, bool isArray){
		auto it = MemoryDebug::map.find((void*)memory);
		if(it!=MemoryDebug::map.end()){
			// Errors like delete new[] or delete[] new
			if(it->second->isArray!=isArray){
				if(!it->second->isArray)
					printf("ERROR: delete[] on new\n");
				else
					printf("ERROR: delete on new[]\n");
				__asm int 3;
			}
		}
		else{
			// If for example a std::vector is created globaly, it's first initialized memory allocation will be done before main starts,
			// this memory will because of this, be unregistred in this memoryDebuger, and this error will occur.
			//printf("ERROR: Your memory cant be deleted, somehow this memory have not been registred in the memoryDebuger\n");
			//__asm int 3;

			return memory;
		}

		isEnabled = false;
		delete it->second;
		map.erase(it);
		isEnabled = true;
		return memory;
	}

	void debugPrint();
};

extern MemoryDebug memoryDebug;

/*
void * __cdecl operator new(size_t size);

void * __cdecl operator new[]( size_t size );

void __cdecl operator delete( void* memory );
void __cdecl operator delete[]( void* memory );*/


#define new memoryDebug(__FILE__,__FUNCTION__,__LINE__) = new


