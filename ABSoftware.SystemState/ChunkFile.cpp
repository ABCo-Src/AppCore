#include "General.h"
#include "ChunkFile.h"

// NOTE: SEE THE COMMENT IN "ChunkFile.cpp" FOR COMPLETE DOCUMENTATION.
// Provides methods for reading/writing a "chunk-based" file. The only "chunk-based" file is the segment files.
// All operations are buffered and performed when "SaveAllChanges" is called.
const int CHUNK_SIZE = 64;
bool ChunkFile::Open(const std::filesystem::path& path)
{
	if (!FileStream::Open(path)) return false;

	// The very first int of the file should be "1" as a safety measure.
	if (ReadInt32() != 1) return false;

	_fileInfoChunk = ReadInt32();
	RootObjectChunk = ReadInt32();
	_freeSpaces = ReadInt32();

	// Load the file info.
	JumpToChunk(_fileInfoChunk);
	{
		_endChunk = FreeChunkPos(ReadInt32());

		uint32_t byteSize = _endChunk.GetBytePos() + 1;
		_allFree.reserve(byteSize);
		ReadBytes((char*)_allFree.data(), byteSize);
	}

	JumpToChunk(RootObjectChunk);
	return true;
}

bool ChunkFile::Create(const std::filesystem::path& path)
{
	if (!FileStream::Create(path)) return false;

	WriteInt32(1);

	// Put the file info object in chunk #1
	WriteInt32(0);
	_fileInfoChunk = 0;

	// Put the root item object in chunk #0
	WriteInt32(1);
	RootObjectChunk = 1;

	// Set the free space to 0.
	WriteInt32(0);

	// Going into the file info chunk after, set the number of chunks to 2.
	WriteInt32(2);

	// Initialize everything as needed
	_freeSpaces = 0;
	_allFree.push_back(0);
	_endChunk = FreeChunkPos(2);

	JumpToChunk(RootObjectChunk);

	return true;
}

uint32_t ChunkFile::CreateObject(uint32_t chunkSize)
{
	// Find a new space that fits this new object.
	FreeChunkPos currentPos = _currentChunkInfoPos = FreeChunkPos();
	while (GoToFreeChunkPos())
	{
		currentPos = _currentChunkInfoPos;

		if (CheckForSequentialBits(chunkSize))
		{
			_freeSpaces -= chunkSize;
			_currentChunkInfoPos = currentPos;
			SetFreeChunkBitsOff(chunkSize);

			return currentPos.Pos;
		}
	}

	// If we weren't able to find any space, just put the object at the end of the file.
	{
		FreeChunkPos objPos = _currentChunkInfoPos = _endChunk;
		_freeSpaces -= SetFreeChunkBitsOff(chunkSize);

		return objPos.Pos;
	}
}

void ChunkFile::DeleteObject(uint32_t chunkPos, uint32_t chunkSize)
{
	_currentChunkInfoPos = FreeChunkPos(chunkPos);
	DeleteObjectAtCurrentPosition(chunkSize);
}

void ChunkFile::DeleteObjectAtCurrentPosition(uint32_t chunkSize)
{
	_freeSpaces += chunkSize;
	SetExistingFreeChunkBitsOn(chunkSize);
}

bool ChunkFile::ResizeObject(uint32_t* chunkPos, uint32_t currentChunks, uint32_t requiredChunks)
{
	if (currentChunks == requiredChunks) return false;

	FreeChunkPos itemChunkPos = FreeChunkPos(*chunkPos);

	// Shrink
	if (requiredChunks < currentChunks)
	{
		uint32_t toGrow = currentChunks - requiredChunks;
		_freeSpaces += toGrow;

		_currentChunkInfoPos = FreeChunkPos(*chunkPos + requiredChunks);
		SetExistingFreeChunkBitsOn(toGrow);
		return false;
	}

	// Grow
	else
	{
		// Ideally, try to keep it where it is, and just expand straight out from there.
		{
			FreeChunkPos beforeScan = _currentChunkInfoPos = FreeChunkPos(*chunkPos + currentChunks);

			uint32_t toGrow = requiredChunks - currentChunks;
			if (CheckForSequentialBits(toGrow))
			{
				_currentChunkInfoPos = beforeScan;

				FreeChunkPos prevEnd = _endChunk;
				_freeSpaces -= SetFreeChunkBitsOff(toGrow);

				return false;
			}
		}

		// If we weren't able to do that, free the old space and then create it like a new object.
		_currentChunkInfoPos = itemChunkPos;

		DeleteObjectAtCurrentPosition(currentChunks);
		*chunkPos = CreateObject(requiredChunks);
		return true;
	}
}

bool ChunkFile::CheckForSequentialBits(int length)
{
	FreeChunkIter<true> iter(this);

	// If we've reached the end of everything, then we'll count that as free space (free at the end of the file).
	if (_currentChunkInfoPos == _endChunk) return true;

	for (; length > 0; length--)
	{
		if (!iter.GetCurrent())
			return false;

		if (!iter.TryMoveNext()) return true;
	}

	return true;
}

// TODO: These could possibly be optimized in the future to do many bits at once. 
int ChunkFile::SetFreeChunkBitsOff(int length)
{
	_beforeChanges.FileInfoChanged = true;

	int start = length;
	FreeChunkIter<true> iter(this);

	// If we've reached the end, go to the end loop instead.
	if (_currentChunkInfoPos == _endChunk) goto LoopAfterEnd;

	for (; length > 0; length--)
	{
		iter.SetCurrentOff();
		if (!iter.TryMoveNext())
		{
			length--;

			iter.MoveNext(); // Force us to move onto the end.
			goto LoopAfterEnd;
		}
	}

	return start;

// A loop that writes once we've gone beyond the end.
LoopAfterEnd:
	int writtenBeforeEnd = start - length;

	// If the end chunk starts in a new byte, create that new byte.
	if (_endChunk.GetBytePos() == _allFree.size())
	{
		GrowFileInfo();
		iter.HandleOverflow();
	}

	for (; length > 0; length--)
	{
		iter.SetCurrentOff();

		// Keep growing the bytes if we need new ones.
		if (!iter.MoveNextFailOverflow())
		{
			GrowFileInfo();
			iter.HandleOverflow();
		}
	}

	_endChunk = _currentChunkInfoPos;
	return writtenBeforeEnd;
}

void ChunkFile::SetExistingFreeChunkBitsOn(int length)
{
	_beforeChanges.FileInfoChanged = true;

	FreeChunkIter<true> iter(this);
	for (; length > 0; length--)
	{
		iter.SetCurrentOn();
		iter.MoveNext();
	}
}

void ChunkFile::JumpToChunk(uint32_t chunkPos)
{
	JumpTo(chunkPos * CHUNK_SIZE + 16);
}

void ChunkFile::JumpToChunk(uint32_t chunkPos, uint32_t offset)
{
	JumpTo(chunkPos * CHUNK_SIZE + 16 + offset);
}

void ChunkFile::GrowFileInfo()
{
	_allFree.push_back(0);
}

void ChunkFile::PrepareForChanges()
{
	_beforeChanges.FileInfoChanged = false;
	_beforeChanges.RootObjectChunk = RootObjectChunk;
	_beforeChanges.FileInfoSize = GetChunksNeededFor(_allFree.size());
	_beforeChanges.FreeSpacesCount = _freeSpaces;
}

void ChunkFile::Flush()
{
	// Apply any changes made to the chunk info.
	if (_beforeChanges.FileInfoChanged)
	{
		bool changedPosition = false;

		int previousSize = 0;
		do
		{
			previousSize = _allFree.size();

			if (ResizeObject(&_fileInfoChunk, _beforeChanges.FileInfoSize, GetChunksNeededFor(_allFree.size())))
				changedPosition = true;

			_beforeChanges.FileInfoSize = previousSize;
		} 		
		// If the chunkSize changed again while trying to position it, try again as it won't have been put 
		// in the right place based on that new chunkSize!
		while (previousSize != _allFree.size());

		// Write all the data
		JumpToChunk(_fileInfoChunk);
		WriteInt32(_endChunk.Pos);
		WriteBytes(_allFree.data(), _allFree.size());

		// Update the file info position, if needed.
		if (changedPosition)
		{
			JumpTo(4);
			WriteInt32(_fileInfoChunk);
		}

		_beforeChanges.FileInfoChanged = 0;
	}

	// Update the root object chunk position.
	if (_beforeChanges.RootObjectChunk != RootObjectChunk)
	{
		JumpTo(8);
		WriteInt32(RootObjectChunk);
	}

	// Update the free space count.
	if (_beforeChanges.FreeSpacesCount != _freeSpaces)
	{
		JumpTo(12);
		WriteInt32(_freeSpaces);
	}
}

// =======
// FREE CHUNK FINDING
// =======
bool ChunkFile::GoToFreeInByte(uint8_t chunk)
{
	if (chunk == 0)
	{
		_currentChunkInfoPos.IncrementToByteStart(1);
		return false;
	}

	FreeChunkIter<false> iter(this);
	do
	{
		if (iter.GetCurrent()) return true;
	} 
	while (iter.TryMoveNext());

	return false;
}

bool ChunkFile::GoToFreeInShort(uint16_t chunk)
{
	if (chunk == 0)
	{
		_currentChunkInfoPos.IncrementToByteStart(2);
		return false;
	}

	char* parts = (char*)&chunk;

	if (GoToFreeInByte(parts[0])) return true;
	return GoToFreeInShort(parts[1]);
}

bool ChunkFile::GoToFreeInInt(uint32_t chunk)
{
	if (chunk == 0)
	{
		_currentChunkInfoPos.IncrementToByteStart(4);
		return false;
	}

	uint16_t* parts = (uint16_t*)&chunk;

	if (GoToFreeInShort(parts[0])) return true;
	return GoToFreeInShort(parts[1]);
}

bool ChunkFile::GoToFreeChunkPos()
{
	char* info = _allFree.data();

	// If we were in the middle of a byte, finish checking the remaining bits in that byte first.
	if (_currentChunkInfoPos.GetBitPos() > 0)
	{
		if (GoToFreeInByte(info[_currentChunkInfoPos.GetBytePos()])) return true;
	}

	// TODO: Support mis-aligned memory better?
	// If our starting place isn't aligned to 8-bytes, we need to work in the largest possible chunks until we can get to a point where it is again.
	//if (_currentChunkInfoPos.GetBytePos % 8 != 0)
	//{
	//	if (_currentChunkInfoPos.GetBytePos % 4 == 0)
	//	{
	//		uint32_t* asInt = (uint32_t*)(info + _currentChunkInfoPos.GetBytePos);
	//		if (GoToFreeInInt(*asInt)) return true;

	//		// We must have gone back in line now.
	//		goto ScanAligned;
	//	}
	//	else if (_currentChunkInfoPos.GetBytePos % 2 == 0)
	//	{
	//		uint16_t* asShort = (uint16_t*)(info + _currentChunkInfoPos.GetBytePos);
	//		if (GoToFreeInShort(*asShort)) return true;

	//		goto Realign;
	//	}
	//	else
	//	{
	//		if (GoToFreeInByte(info[_currentChunkInfoPos.GetBytePos])) return true;
	//		goto Realign;
	//	}
	//}
	
	int toGo = _allFree.size() - _currentChunkInfoPos.GetBytePos();

	// Check in as many large chunks as possible.
	while (toGo >= 8)
	{
		uint64_t* asLong = (uint64_t*)(info + _currentChunkInfoPos.GetBytePos());
		toGo -= 8;

		if (*asLong == 0)
		{
			_currentChunkInfoPos.IncrementToByteStart(8);
			continue;
		}

		uint32_t* parts = (uint32_t*)asLong;

		if (GoToFreeInInt(parts[0])) return true;
		if (GoToFreeInInt(parts[1])) return true;

	}

	if (toGo >= 4)
	{
		uint32_t* asInt = (uint32_t*)(info + _currentChunkInfoPos.GetBytePos());
		if (GoToFreeInInt(*asInt)) return true;

		toGo -= 4;
	}

	if (toGo >= 2)
	{
		uint16_t* asShort = (uint16_t*)(info + _currentChunkInfoPos.GetBytePos());
		if (GoToFreeInShort(*asShort)) return true;

		toGo -= 2;
	}

	if (toGo == 1)
		if (GoToFreeInByte(info[_currentChunkInfoPos.GetBytePos()])) return true;

	return false;
}

uint32_t ChunkFile::GetChunksNeededFor(uint32_t size)
{
	uint32_t chunks = size / CHUNK_SIZE;

	// Round up the result.
	if (size % CHUNK_SIZE > 0)
		chunks++;

	return chunks;
}