#include "General.h"
#include "Segment.h"
#include "Saving.h"

// Shared for reading/changes, not shared for adding/removing.
static std::shared_mutex LoadedSegmentsLock;
static std::vector<RegSegment*> LoadedSegments;

// SEGMENT STRUCTURE
// ---------------
// The groups contain all of their items stored right inline. And any sub-arrays or sub-groups
// are simply references to elsewhere.
// 
// When a sub-array or sub-group within a group is needed, that item and all of its sub-items are loaded.
// The exact structure of a group is like so ("|" is just used a seperator for the diagram):
//
// ================================================================================
// Unused | No Sub-Arrays (1 byte) | No Sub-Groups (1 byte) | No Sub-Items (1 byte) | 
// Sub-Arrays (Name - 4 bytes, Actual Array - 4 bytes) | Sub-Groups (Name - 4 bytes, Actual Array - 4 bytes) | Sub-Items (Name - 4 bytes, Actual Item - 8 bytes)
// ================================================================================
RegSegment* SegmentHandler::GetOrLoadSegment(std::string& name)
{
	// Attempt to get the segment.
	{
		LoadedSegmentsLock.lock_shared();

		for (uint32_t i = 0; i < LoadedSegments.size(); i++)
			if (LoadedSegments[i]->Name == name)
				return LoadedSegments[i];

		LoadedSegmentsLock.unlock_shared();
	}

	// If not loaded, load it now
	RegSegment* segment = LoadSegment(name);
	if (segment == nullptr) return nullptr;

	// Place the segment into the loaded segments
	{
		LoadedSegmentsLock.lock();
		LoadedSegments.push_back(segment);
		LoadedSegmentsLock.unlock();
	}

	return segment;
}

void SegmentHandler::SaveAllSegments()
{
	uint32_t i = 0;

	while (true)
	{
		// Get the next segment
		RegSegment* segment;
		{
			LoadedSegmentsLock.lock_shared();

			if (++i >= LoadedSegments.size()) break;
			segment = LoadedSegments[i];

			LoadedSegmentsLock.unlock_shared();
		}

		// SaveAllChanges this segment
		//segment->SaveAllChanges();
	}

}

RegSegment::RegSegment()
{
	File = new RegSegmentFileOperator(this);
}

void RegSegment::Save()
{
	// Don't save it if it hasn't changed.
	if (File->AddedChanges.Size > 0 || File->ModifiedGroups.Size > 0 || File->ModifiedArrays.Size > 0) return;

	File->SaveAllChanges();
}

void RegSegmentFileOperator::SaveAllChanges()
{
	// Get all the changes, clearing it out for the users.
	_currentModifiedGroups = GetNewChangesBuffer<DataPosition>(_currentModifiedGroups, ModifiedGroups);
	_currentModifiedArrays = GetNewChangesBuffer<DataPosition>(_currentModifiedArrays, ModifiedArrays);
	_currentAddedObjects = GetNewChangesBuffer<AddChangesItem>(_currentAddedObjects, AddedChanges);

	std::vector<uint32_t> groupsToProcessAfter;

	File.PrepareForChanges();

	// Process added items first - the parents of these new items have already been added to the "modified"
	// and as such don't need to be manually tweaked by this.
	SaveAdded();

	// Process modified arrays.
	for (int i = 0; i < _currentModifiedArrays.Size; i++)
	{
		DataPosition pos = _currentModifiedArrays.Array[i];

		RegArray& arr = Parent->LoadedArrays.Get(pos);
		UpdateArray(pos, arr);
	}

	// Process modified groups.
	for (int i = 0; i < _currentModifiedGroups.Size; i++)
	{
		DataPosition pos = _currentModifiedGroups.Array[i];

		RegGroup& group = Parent->LoadedGroups.Get(pos);
		UpdateGroup(pos, group);
	}

	File.Flush();

	HasDeletedItemsSinceLast = false;
}

// ===========
// LOADING
// ===========
RegSegment* SegmentHandler::LoadSegment(std::string& name)
{
	RegSegment* res = new RegSegment();

	if (!res->Initialize(name))
	{
		delete res;
		return nullptr;
	}

	return res;
}

bool RegSegment::Initialize(const std::string& name)
{
	return File->Initialize(SegmentsDirectory / name);
}

void RegSegmentFileOperator::Create(const std::filesystem::path& name)
{
	File.Create(name);

	RegGroup* rootGroup = CreateRootGroupOutline();

	// Now write out that the root group has 0 of each thing in it.
	File.WriteInt32(0);
}

bool RegSegmentFileOperator::Initialize(const std::filesystem::path& name)
{
	File.Open(name);

	RegGroup* rootGroup = CreateRootGroupOutline();
	LoadGroup(*rootGroup);
	return true;
}

RegGroup* RegSegmentFileOperator::CreateRootGroupOutline()
{
	// Create the root group
	RegGroup* rootGroup = &Parent->LoadedGroups.Emplace(Parent->RootGroup);
	rootGroup->Name = DataPosition(65535, 255); // There is no name.
	rootGroup->CurrentFileChunk = File.RootObjectChunk;

	return rootGroup;
}

// ===========
// Groups
// ===========
bool RegSegmentFileOperator::LoadGroup(RegGroup& group)
{
	File.JumpToChunk(group.CurrentFileChunk);

	// In big endian terms:
	// Unused | Sub-Array Count (1 byte) | Sub-Group Count (1 byte) | Sub-Item Count (1 byte) 
	uint32_t header = File.ReadInt32();

	uint8_t subArrayCount = header >> 24;
	uint8_t subGroupCount = header >> 16; 
	uint8_t subItemCount = header >> 8;

	// Read sub-arrays.
	group.SubArrays.reserve(subArrayCount);
	for (uint8_t i = 0; i < subArrayCount; i++)
		group.SubArrays[i] = ReadAndCreateUnloadedObject<LoadedArraysContainer>(Parent->LoadedArrays);

	// Read sub-groups.
	group.SubGroups.reserve(subGroupCount);
	for (uint8_t i = 0; i < subGroupCount; i++)
		group.SubGroups[i] = ReadAndCreateUnloadedObject<LoadedGroupsContainer>(Parent->LoadedGroups);

	// Read sub-items.
	group.SubItems.reserve(subItemCount);
	for (uint8_t i = 0; i < subItemCount; i++)
		if (!LoadSimpleItem(group.SubGroups[i])) 
			return false;

	group.IsLoaded = true;
	return true;
}

uint32_t RegSegmentFileOperator::GetGroupDataSize(RegGroup& obj)
{
	return 4 + obj.SubArrays.size() * 4 + obj.SubGroups.size() * 4 + obj.SubItems.size() * 12;
}

void RegSegmentFileOperator::SaveGroupData(RegGroup& obj)
{
	// In big endian terms:
	// Unused | Sub-Array Count (1 byte) | Sub-Group Count (1 byte) | Sub-Item Count (1 byte) 
	uint32_t header = 0;

	header |= obj.SubArrays.size() >> 24;
	header |= obj.SubGroups.size() >> 16;
	header |= obj.SubItems.size() >> 8;

	File.WriteInt32(header);

	// Write sub-arrays.
	for (uint8_t i = 0; i < obj.SubArrays.size(); i++)
		obj.SubArrays[i] = ReadAndCreateUnloadedObject<LoadedArraysContainer>(Parent->LoadedArrays);

	// Write sub-groups.
	for (uint8_t i = 0; i < obj.SubGroups.size(); i++)
		obj.SubGroups[i] = ReadAndCreateUnloadedObject<LoadedGroupsContainer>(Parent->LoadedGroups);

	// Write sub-items.
	for (uint8_t i = 0; i < obj.SubItems.size(); i++)
		SaveSimpleItemData(obj.SubItems[i]);
}

// ===========
// Arrays
// ===========
bool RegSegmentFileOperator::LoadArray(RegArray& arr)
{
	File.JumpToChunk(arr.CurrentFileChunk);
	
	// In big endian terms:
	// Type (1 byte) | Size (3 bytes)
	uint32_t header = File.ReadInt32();
	arr.ItemType = (RegItemType)(header >> 24);

	if ((char)arr.ItemType > 5) return false;

	uint32_t length = header & 0b00000000111111111111111111111111;

	if (arr.ItemType == RegItemType::Char)
	{
		RegString& str = (RegString&)arr;
		str.String.reserve(length);
		File.ReadBytes(str.String.data(), length);
	}
	else
	{
		RegNonStringArray& other = (RegNonStringArray&)arr;

		other.Items.reserve(length);
		for (uint32_t i = 0; i < length; i++)
			if (!LoadSimpleItem(other.Items[i])) return false;
	}

	arr.IsLoaded = true;
	return true;
}

uint32_t RegSegmentFileOperator::GetArrayDataSize(RegArray& arr)
{
	if (arr.ItemType == RegItemType::Char)
		return ((RegString&)arr).String.size();

	RegNonStringArray& otherArr = (RegNonStringArray&)arr;
	return otherArr.Items.size() * ((otherArr.ItemType == RegItemType::String) ? 4 : 8);
}

void RegSegmentFileOperator::SaveArrayData(RegArray& arr)
{
	arr.HasChanged = false;

	// String
	if (arr.ItemType == RegItemType::Char)
	{
		RegString& str = (RegString&)arr;
		File.WriteInt32(((uint8_t)RegItemType::Char << 24) | str.String.size());
		File.WriteBytes(str.String.data(), str.String.size());
	}

	// Other
	else
	{
		RegNonStringArray& otherArr = (RegNonStringArray&)arr;
		uint32_t header = ((uint8_t)arr.ItemType << 24) | otherArr.Items.size();
		
		// Sub-strings
		if (arr.ItemType == RegItemType::String)
			for (size_t i = 0; i < otherArr.Items.size(); i++)
				SaveObjectPos<RegArray>(otherArr.Items[i], Parent->LoadedArrays);

		// Sub-items
		else
			for (size_t i = 0; i < otherArr.Items.size(); i++)
				SaveSimpleItemData(otherArr.Items[i]);
	}
}

// ===========
// Items
// ===========
bool RegSegmentFileOperator::LoadSimpleItem(DataPosition& res)
{
	RegSimpleItem& dest = Parent->LoadedItems.Emplace(res);
	dest.Name = ReadAndCreateUnloadedObject<RegArray>(Parent->LoadedArrays);
	dest.Type = (RegItemType)File.ReadByte();

	if ((char)dest.Type > 3) return false;

	switch (dest.Type)
	{
	case RegItemType::Integer:
	case RegItemType::BlGroup:
		dest.Integer = File.ReadInt64();
		break;
	case RegItemType::Decimal:
		dest.Decimal = File.ReadDouble();
		break;
	}

	return true;
}

uint32_t RegSegmentFileOperator::GetItemDataSize(RegSimpleItem& item)
{
	return 8;
}

void RegSegmentFileOperator::SaveSimpleItemData(DataPosition pos)
{
	RegSimpleItem& item = Parent->LoadedItems.Get(pos);

	switch (item.Type)
	{
	case RegItemType::Integer:
	case RegItemType::BlGroup:
		File.WriteInt64(item.Integer);
		break;
	case RegItemType::Decimal:
		File.WriteDouble(item.Decimal);
		break;
	}
}

//void RegSegment::ParseLookup(RegLookup& dest)
//{
//	dest.IsLoaded = false;
//
//	// Group Name
//	dest.NameLen = File.ReadByte();
//	dest.Name = new char[dest.NameLen];
//	File.ReadBytes(dest.Name, dest.NameLen);
//
//	// Pos position
//	dest.Place = File.ReadInt32();
//}
//
//void RegSegment::TrackChange(DataPosition pos)
//{
//
//}

void RegSegmentFileOperator::UpdateArray(DataPosition pos, RegArray& arr)
{
	if (!arr.HasChanged) return;
	if (ShouldSkipDeleted(pos, AddChangesType::Array)) return;
	
	std::unique_lock<std::shared_mutex> guard(arr.ModifyLock);
	HandleResizeObjectChunk(pos, arr, GetArrayDataSize(arr), true);
	SaveArrayData(arr);
}

void RegSegmentFileOperator::UpdateGroup(DataPosition pos, RegGroup& group)
{
	if (!group.HasChanged) return;
	if (ShouldSkipDeleted(pos, AddChangesType::Group)) return;

	std::unique_lock<std::shared_mutex> guard(group.ModifyLock);
	HandleResizeObjectChunk(pos, group, GetGroupDataSize(group), false);
	SaveGroupData(group);
}

void RegSegmentFileOperator::HandleResizeObjectChunk(DataPosition objPos, RegObject& obj, uint32_t newSize, bool isArray)
{
	bool moved = File.ResizeObject(&obj.CurrentFileChunk, obj.CurrentFileChunkLength, newSize);
	File.JumpToChunk(obj.CurrentFileChunk);

	// If it moved we need to update the reference in the parent to match.
	if (moved)
	{
		RegObject& parent = obj.IsParentArray ? (RegObject&)Parent->LoadedArrays.Get(obj.Parent) : (RegObject&)Parent->LoadedGroups.Get(obj.Parent);

		// Lock to make sure it doesn't change as we look at it.
		parent.ModifyLock.lock();

		// If the parent has changes that haven't been saved yet, or it's changed since the last time
		// we saved it, we'll re-save the whole thing now.
		//
		// If it has already been saved and hasn't changed since then, simply modify ("tweak") the
		// item already located within the output.
		if (obj.IsParentArray)
		{
			if (parent.HasChanged)
			{
				parent.ModifyLock.unlock();
				UpdateArray(obj.Parent, (RegNonStringArray&)parent);
				return;
			}

			TweakChildPosInArray(obj.Parent, obj.CurrentFileChunk, (RegNonStringArray&)parent);
		}
		else
		{
			if (parent.HasChanged)
			{
				parent.ModifyLock.unlock();
				UpdateGroup(obj.Parent, (RegGroup&)parent);
				return;
			}

			if (isArray)
				TweakArrayChildPosInGroup(obj.Parent, obj.CurrentFileChunk, (RegGroup&)parent);
			else
				TweakGroupChildPosInGroup(obj.Parent, obj.CurrentFileChunk, (RegGroup&)parent);
		}

		parent.ModifyLock.unlock();
	}
}

uint32_t FindPosInVector(std::vector<DataPosition>& vec, DataPosition target)
{
	for (uint32_t i = 0; i < vec.size(); i++)
		if (vec[i] == target)
			return i;

	Crash("Unable to find item in vector that should be unchanged!");
	return -1;
}

void RegSegmentFileOperator::TweakChildPosInArray(DataPosition oldPos, uint32_t newPos, RegNonStringArray& arr)
{
	uint32_t pos = FindPosInVector(arr.Items, oldPos);

	File.JumpToChunk(arr.CurrentFileChunk, 4 + pos * 4);
	File.WriteInt32(newPos);
}

void RegSegmentFileOperator::TweakGroupChildPosInGroup(DataPosition oldPos, uint32_t newPos, RegGroup& group)
{
	uint32_t pos = FindPosInVector(group.SubGroups, oldPos);

	File.JumpToChunk(group.CurrentFileChunk, 4 + (size_t)pos * 4);
	File.WriteInt32(newPos);
}

void RegSegmentFileOperator::TweakArrayChildPosInGroup(DataPosition oldPos, uint32_t newPos, RegGroup& group)
{
	uint32_t pos = FindPosInVector(group.SubArrays, oldPos);

	File.JumpToChunk(group.CurrentFileChunk, 4 + group.SubGroups.size() * 4 + (size_t)pos * 4);
	File.WriteInt32(newPos);
}

void RegSegmentFileOperator::SaveAdded()
{
	for (int i = 0; i < _currentAddedObjects.Size; i++)
	{
		AddChangesItem item = _currentAddedObjects.Array[i];
		if (ShouldSkipDeleted(item.Pos, item.Type)) continue;

		if (item.Type == AddChangesType::Array)
		{
			RegArray& arr = Parent->LoadedArrays.Get(item.Pos);

			std::unique_lock<std::shared_mutex> guard(arr.ModifyLock);

			arr.CurrentFileChunkLength = File.GetChunksNeededFor(GetArrayDataSize(arr));
			arr.CurrentFileChunk = File.CreateObject(arr.CurrentFileChunkLength);

			File.JumpToChunk(arr.CurrentFileChunk);
			SaveArrayData(arr);
		}
		else
		{
			RegGroup& group = Parent->LoadedGroups.Get(item.Pos);

			std::unique_lock<std::shared_mutex> guard(group.ModifyLock);

			group.CurrentFileChunkLength = File.GetChunksNeededFor(GetGroupDataSize(group));
			group.CurrentFileChunk = File.CreateObject(group.CurrentFileChunkLength);

			File.JumpToChunk(group.CurrentFileChunk);
			SaveGroupData(group);
		}
	}
}

bool RegSegmentFileOperator::ShouldSkipDeleted(uint32_t pos, AddChangesType type)
{
	if (!HasDeletedItemsSinceLast) return false;

	// See if this item was deleted.
	auto setPos = DeletedItemsSinceLast.find(pos);
	if (setPos == DeletedItemsSinceLast.end()) return false;

	return setPos->second == type;
}