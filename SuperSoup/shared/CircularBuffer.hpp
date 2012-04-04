//non-blocking-queue, ideal for one producer and one consumer, can r/w at the same time without blocking.
//take caution when using this class.
//some things to read about:
//CAS compare and swap atomic operation.
//volatile keyword assures that there is no reordering when compiler does optimization, assures that a write is updated to main memory instead of just stored in some register

#pragma once

template<typename T>
class CircularBuffer
{
private:
	unsigned int capacity;
	T* itemArray;

	volatile unsigned int start; //index of oldest element
	volatile unsigned int end; //index at which to write new element
	
public:
	void construct(unsigned int capacity)
	{
		this->capacity = capacity;
		itemArray = new T[capacity];

		start = 0;
		end = 0;
	}

	void destruct()
	{
		delete[] itemArray;
	}

	bool isFull()
	{
		return (end + 1) % capacity == start;
	}

	void addItem(const T& item)
	{
		//caller must make sure isFull is false
		//otherwise it will overwrite existing items
		itemArray[end] = item;
		end = (end + 1) % capacity;
	}

	bool isEmpty()
	{		
		return start == end;
	}

	T popItem()
	{
		//caller must make sure isEmpty is false
		//otherwise it will read wrong items
		T item = itemArray[start];
		start = (start + 1) % capacity;
		return item;
	}
};
