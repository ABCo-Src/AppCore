#pragma once
#include "General.h"

// NOTE: All "|"s below are just purely for diagram purposes.
// 
// In a chunk file, everything is structured a little like a simple heap. The file is split into "data chunks", which are 64-byte blocks of data.
// Each chunk is either free (empty) or contains an "object". A object is just a block of data that stores one thing.
// When a data chunk is filled, it will only ever contain one object, you can't have two objects together in one data chunk. It's one thing per chunk.
// If an object is too big to fit into a chunk, it will span across multiple continuous chunks, like this:
//
// ======== ======== ======== ======== (Pos chunks)
// <------> <---------------> <------>
// Group  1 Group           2 Group  3
//
//
// START OF FILE
// -------------------
//
// At the start of the file some important information is stored, structured as below:
//
// ===================================
// 1 Safety | 2 'Chunk Info Object' | 3 'Root Object' | 4 No of Free Chunks
// ===================================
//
// 1. The safety number is a 32-bit integer that at the moment will ALWAYS be 1. This can be used in the future if any changes are made to the format that need to be read differently.
// 2. This is the chunk where the "chunk info" object is located. (More details below). 32-bit integer
// 3. This is the chunk where the "root" object is located. In a segment file, this is the root group. 32-bit integer.
// 4. How many free chunks are currently available in the file. 32-bit integer.
//
// 
// CHUNK INFO OBJECT
// -------------------
//
// One of the "objects" you can get is the "chunk info" object, which stores all the data important info about the file. There is only one of these per file.
// That object is structured like so):
//
// ===================================
// 1 # Of Chunks | 2 Chunks Statuses
// ===================================
// 1. A 32-bit integer representing how many chunks there are in total.
// 2. A set of bytes, where each bit sequentially represents whether a chunk has data or not. ON represents a free chunk and OFF represents a filled chunk.
//
// All of the different types of objects you can normally get in a segment file are: Group, Array and file info.
class ChunkFile : public FileStream
{
private:
	class FreeChunkPos
	{
	public:
		uint32_t Pos;

		uint32_t GetBytePos() { return Pos >> 3; }
		uint8_t GetBitPos() { return Pos & 0b111; }
		void IncrementToByteStart(int amount) { Pos &= ~0b111; Pos += amount * 8; }
		void operator++(int) { Pos++; }
		void operator--(int) { Pos--; }
		FreeChunkPos operator+(uint32_t right) { return FreeChunkPos(Pos + right); }
		FreeChunkPos operator-(uint32_t right) { return FreeChunkPos(Pos - right); }
		bool operator==(const FreeChunkPos& right) { return Pos == right.Pos; }

		FreeChunkPos() { Pos = 0; }
		FreeChunkPos(uint32_t data) { Pos = data; }
	};

	// Helper Iterator
	template<bool OverflowToNextByte>
	class FreeChunkIter
	{
	private:
		ChunkFile* _parent = nullptr;
		char* _currentByte = nullptr;
		FreeChunkPos _lastPos;
		uint16_t _currentMask = 0;

	public:
		FreeChunkIter(ChunkFile* parent)
		{
			_parent = parent;
			_currentByte = parent->_allFree.data() + parent->_currentChunkInfoPos.GetBytePos();
			_currentMask = 128 >> parent->_currentChunkInfoPos.GetBitPos();
			_lastPos = parent->_endChunk - 1;
		}

		// Tries to move next and fails if we've reached the end of the byte or strig.
		bool TryMoveNext()
		{
			// Fail if we've reached the end of the current byte.
			if constexpr (!OverflowToNextByte)
				if (AtByteEnd())
					return false;

			// If we're at the last position now, we're going to stop.
			if (_parent->_currentChunkInfoPos == _lastPos) return false;

			MoveNext();
			return true;
		}

		inline void MoveNext()
		{
			if (!MoveNextFailOverflow())
				HandleOverflow();
		}

		inline bool MoveNextFailOverflow()
		{
			_parent->_currentChunkInfoPos++;

			if (AtByteEnd())
				return false;

			_currentMask >>= 1;
			return true;
		}

		bool AtByteEnd()
		{
			return _currentMask == 1;
		}

		void HandleOverflow()
		{
			_currentMask = 128;
			_currentByte = _parent->_allFree.data() + _parent->_currentChunkInfoPos.GetBytePos();
		}

		bool GetCurrent()
		{
			return (*_currentByte & _currentMask) > 0;
		}

		void SetCurrentOff()
		{
			*_currentByte &= ~_currentMask;
		}

		void SetCurrentOn()
		{
			*_currentByte |= _currentMask;
		}
	};

	class BeforeChangesState
	{
	public:
		bool FileInfoChanged = false;
		int FileInfoSize = 0;
		int RootObjectChunk = 0;
		int FreeSpacesCount = 0;
	};

	BeforeChangesState _beforeChanges = BeforeChangesState();

	// Tracks where we currently are in the "chunk info" chunk.
	FreeChunkPos _currentChunkInfoPos = FreeChunkPos(0);

	// The number of free spaces before the "_endChunk".
	int _freeSpaces = 0;

	// ChunkInfo
	uint32_t _fileInfoChunk = 0;
	FreeChunkPos _endChunk = FreeChunkPos(0);
	std::vector<char> _allFree;

	void GrowFileInfo();

	bool CheckForSequentialBits(int length);

	// Returns: How many existing bits were set to off.
	int SetFreeChunkBitsOff(int length);
	void SetExistingFreeChunkBitsOn(int length);

	// GoToFreeChunkPos:
	bool GoToFreeInByte(uint8_t data);
	bool GoToFreeInShort(uint16_t chunk);
	bool GoToFreeInInt(uint32_t chunk);
	bool GoToFreeChunkPos();

	void DeleteObjectAtCurrentPosition(uint32_t chunkSize);

public:
	uint32_t RootObjectChunk = 0;

	bool Create(const std::filesystem::path& data);
	bool Open(const std::filesystem::path& data);

	void PrepareForChanges();
	void Flush();

	void JumpToChunk(uint32_t chunkPos);
	void JumpToChunk(uint32_t chunkPos, uint32_t offset);
	uint32_t GetChunksNeededFor(uint32_t size);

	// Returns whether the position changed.
	bool ResizeObject(uint32_t* chunkPos, uint32_t previousChunkSize, uint32_t expectedChunkSize);
	uint32_t CreateObject(uint32_t chunkSize);
	void DeleteObject(uint32_t chunkPos, uint32_t chunkSize);
};