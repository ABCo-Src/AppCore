#pragma once
#include "Items.h"
#include "ChunkFile.h"
#include "Segment.h"

enum class AddChangesType : byte
{
	Group,
	Array
};

// An item in a collection of changes of items added or deleted.
struct AddChangesItem
{
	AddChangesType Type = AddChangesType::Group;
	DataPosition Pos;
};

class RegSegment;

template<typename T>
class SegmentChangesTracker
{
public:
	static const int DEFAULT_CAPACITY = 32;
	int Size = 0;

	bool Add(DataPosition pos)
	{
		std::lock_guard<std::mutex> lock(_lock);
		int newPosition = Size++;

		// If we're starting to get close to the capacity, we need to warn the system that we need 
		// to save so we don't fill the buffer (which blocks everything).
		if (newPosition > _data.Size - 8)
		{
			if (Size == _data.Size)
			{
				// We have to reallocate to keep moving unfortunately. The save process should be on its way soon.
				DataPosition* oldArr = _data.Array;
				_data = SizedArray<DataPosition>(_data.Size * 2);
				delete[] oldArr;
			}

			else _data.Array[newPosition] = pos;

			return true;
		}

		_data.Array[newPosition] = pos;
		return false;
	}

	SizedArray<T> SwapBuffer(SizedArray<T> swapWith)
	{
		std::lock_guard<std::mutex> lock(_lock);

		SizedArray<T> before = _data;
		_data = swapWith;
		Size = 0;

		return before;
	}
private:
	std::mutex _lock;
	SizedArray<T> _data = SizedArray<T>(DEFAULT_CAPACITY);
};

// The object responsible for handling all saving to disk in a segment.
class RegSegmentFileOperator
{
private:
	SizedArray<DataPosition> _currentModifiedGroups;
	SizedArray<DataPosition> _currentModifiedArrays;
	SizedArray<AddChangesItem> _currentAddedObjects;

public:
	// ============
	// Loading
	// ============
	bool LoadSimpleItem(DataPosition& res);

	// ============
	// Saving
	// ============
	bool ShouldSkipDeleted(uint32_t pos, AddChangesType type);

	void TweakChildPosInArray(DataPosition oldPos, uint32_t newPos, RegArray& arr);
	void TweakArrayChildPosInGroup(DataPosition oldPos, uint32_t newPos, RegGroup& group);
	void TweakGroupChildPosInGroup(DataPosition oldPos, uint32_t newPos, RegGroup& group);

	uint32_t GetItemDataSize(RegSimpleItem& item);
	void SaveSimpleItemData(DataPosition pos);

	bool LoadArray(RegArray& dest);
	uint32_t GetArrayDataSize(RegArray& arr);
	void SaveArrayData(RegArray& arr);

	bool LoadStr(RegString& dest);
	uint32_t GetStringDataSize(RegString& str);
	void SaveStringData(RegString& str);

	bool LoadGroup(RegGroup& dest);
	uint32_t GetGroupDataSize(RegGroup& obj);
	void SaveGroupData(RegGroup& obj);

	void UpdateArray(DataPosition pos, RegArray& arr);
	void UpdateGroup(DataPosition pos, RegGroup& group);

	void HandleResizeObjectChunk(DataPosition objPos, RegObject& obj, uint32_t newSize, bool isArray);
	void SaveAdded();

	// ============
	// Templated Helpers
	// ============

	template<typename Container>
	DataPosition ReadAndCreateUnloadedObject(Container& containerToCreateInto)
	{
		DataPosition namePos;
		RegArray& name = Parent->LoadedArrays.Emplace(namePos);
		name.CurrentFileChunk = File.ReadInt32();
		name.IsLoaded = false;

		DataPosition res;

		RegObject& obj = containerToCreateInto.Emplace(res);
		obj.Name = namePos;
		obj.CurrentFileChunk = File.ReadInt32();
		obj.IsLoaded = false;

		return res;
	}

	template<typename Container>
	void SaveObjectPos(DataPosition pos, Container& objContainer)
	{
		File.WriteInt32(((RegObject&)objContainer.Get(pos)).CurrentFileChunk);
	}

	template<typename T>
	SizedArray<T> GetNewChangesBuffer(SizedArray<T>& potentialCandidate, SegmentChangesTracker<T>& swapWith)
	{
		SizedArray<T> newBuffer = potentialCandidate.Array == nullptr
			? SizedArray<T>(SegmentChangesTracker<T>::DEFAULT_CAPACITY) : potentialCandidate;

		return swapWith.SwapBuffer(newBuffer);
	}

public:
	SegmentChangesTracker<DataPosition> ModifiedGroups;
	SegmentChangesTracker<DataPosition> ModifiedArrays;
	SegmentChangesTracker<AddChangesItem> AddedChanges;

	RegSegment* Parent = nullptr;

	ChunkFile File;
	std::mutex FileLock;

	bool HasDeletedItemsSinceLast = false;
	std::unordered_map<uint32_t, AddChangesType> DeletedItemsSinceLast;
	char* WriteBuffer = nullptr;

	void Create(const std::filesystem::path& path);
	bool Initialize(const std::filesystem::path& path);
	RegGroup* CreateRootGroupOutline();
	void SaveAllChanges();

	RegSegmentFileOperator(RegSegment* parent) { Parent = parent; }
};

// Provide hashing for "AddChangesItem".
namespace std
{
	template<>
	struct hash<AddChangesItem> {
		size_t operator()(const AddChangesItem& i) const
		{
			return ((size_t)i.Type ^ ((size_t)i.Pos << 1)) >> 1;
		}
	};
}