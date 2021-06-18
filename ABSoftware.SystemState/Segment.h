#pragma once
#include "Items.h"
#include "CoreDataContainer.h"
#include "ChunkFile.h"

class RegSegmentFileOperator;

typedef CoreDataContainer<RegSimpleItem, 64> LoadedItemsContainer;

// Since strings are used in the names of everything we expect them to be very common
typedef CoreDataContainer<RegString, 128> LoadedStringsContainer;
typedef CoreDataContainer<RegGroup, 16> LoadedGroupsContainer;
typedef CoreDataContainer<RegArray, 8> LoadedArraysContainer;

class RegSegment
{
public:
	std::string Name;
	bool CurrentlySaving = false;

	RegSegmentFileOperator* File;

	// The chunk sizes were chosen based on how common these are likely to appear.
	LoadedItemsContainer LoadedItems;
	LoadedStringsContainer LoadedStrings;
	LoadedGroupsContainer LoadedGroups;
	LoadedArraysContainer LoadedArrays;
	DataPosition RootGroup;

	bool Initialize(const std::string& name);

	RegString& GetLoadedStrName(DataPosition pos);
	RegString& GetLoadedArrayName(DataPosition pos);
	RegString& GetLoadedGroupName(DataPosition pos);
	RegString& GetLoadedItemName(DataPosition pos);

	RegString& GetLoadedStr(DataPosition pos);
	RegArray& GetLoadedArray(DataPosition pos);
	RegGroup& GetLoadedGroup(DataPosition pos);
	RegSimpleItem& GetLoadedItem(DataPosition pos);

	void TrackChange(DataPosition pos);

	void Save();

	RegSegment();
private:
	RegString& GetLoadedObjName(RegObject& obj);
};

namespace SegmentHandler
{
	RegSegment* GetOrLoadSegment(std::string& name);
	RegSegment* LoadSegment(std::string& name);

	// NOTE: This is expected to be running on just one thread at any given time.
	void SaveAllSegments();
}