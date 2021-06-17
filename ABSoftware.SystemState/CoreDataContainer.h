#pragma once
#include "General.h"

class DataPosition
{
private:
	uint32_t _data = 0;
public:
	uint16_t GetChunk() { return (uint16_t)_data; }
	uint8_t GetIndex() { return _data >> 24; }

	DataPosition()
	{
		_data = 0;
	}

	DataPosition(uint16_t chunk, uint8_t index)
	{
		_data = ((uint32_t)chunk << 16) | index;
	}

	operator uint32_t() const { return _data; }
};

// The central, core object responsible for storing all loaded data.
// This achieves the following:
//
// Fixed - Once an item has been placed a given position, it will always remain in that position. Even when items get removed or added around it.
// Concurrent - This is designed to be accessible from many different threads at a given time.
// Non re-allocating - If the capacity is reached, the existing data is NOT re-allocated and moved. Instead, a new "chunk" is added, completely seperate to the existing data.
// Fast - This is designed to be fast, as it is the absolute core of the registry. It has a very high priority on individual access times, as opposed to addition and removal, which are low-priority operations.
template<typename T, int ChunkSize>
class CoreDataContainer
{
private:
	std::shared_mutex _lock;

	class DataChunk
	{
	public:
		T LookupData[ChunkSize];

		// Protected by "_addRemLock":
		uint8_t DataFilled = 0;
	};

	// Protected by "_addRemLock":
	std::queue<DataPosition> _freeMiddleSpaces;

	// Protected for writing by "_addRemLock":
	bool _chunkNeedsToChange = false;
	int _chunksCapacity = 0;
	int _chunksLastIdx = 0;

	// Protected by "_chunksLock":
	std::vector<DataChunk*> _chunks;

	void GrowChunks()
	{
		_chunks.push_back(new DataChunk());
	}

public:

	T& Get(DataPosition pos)
	{
		T* val = TryGet(pos);
		if (val == nullptr) Crash("Attempted to get item out of bounds of CoreDataContainer.");
		return *val;
	}

	// Returns nullptr if item was out-of-bounds.
	T* TryGet(DataPosition pos)
	{
		// Get the chunk safely.
		DataChunk* containingChunk;
		{
			if (pos.GetChunk() > _chunksLastIdx) return nullptr;

			_lock.lock_shared();
			containingChunk = _chunks[pos.GetChunk()];
			_lock.unlock_shared();
		}

		// Get the data from it.
		return &containingChunk->LookupData[pos.GetIndex()];
	}

	DataPosition Add(T& toAdd)
	{
		DataPosition res;
		Emplace(res) = toAdd;
		return res;
	}
	
	T& Emplace(DataPosition& pos)
	{
		// TODO: Try to improve addition times here.
		std::lock_guard<std::shared_mutex> lock(_lock);

		// If there's no free spaces in the middle, add to the end.
		if (_freeMiddleSpaces.empty())
		{
			bool needsNewChunk = _chunks[_chunksLastIdx]->DataFilled == ChunkSize;

			// Allocate a new chunk if necessary.
			if (needsNewChunk)
			{
				if (++_chunksLastIdx == _chunksCapacity) GrowChunks();

				// Add the new chunk
				DataChunk* newChunk = _chunks[_chunksLastIdx] = new DataChunk();
				pos = DataPosition(_chunksLastIdx, 0);
				return newChunk->LookupData[newChunk->DataFilled++];
			}

			else
			{
				pos = DataPosition(_chunksLastIdx, _chunks[_chunksLastIdx]->DataFilled);
				return _chunks[_chunksLastIdx]->LookupData[_chunks[_chunksLastIdx]->DataFilled++];
			}
		}

		// Add to the middle space
		else
		{
			DataPosition newPos = _freeMiddleSpaces.front();
			pos = newPos;

			_freeMiddleSpaces.pop();
			return _chunks[newPos.GetChunk()]->LookupData[newPos.GetIndex()];
		}
	}

	void Remove(DataPosition pos)
	{
		std::lock_guard<std::shared_mutex> lock(_lock);
		_freeMiddleSpaces.push(pos);
	}

	CoreDataContainer()
	{
		GrowChunks();
	}

	~CoreDataContainer()
	{
		for (int i = 0; i < _chunksLastIdx; i++)
			delete[] _chunks[_chunksLastIdx];
	}
};