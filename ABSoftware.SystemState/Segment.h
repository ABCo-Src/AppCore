#pragma once
#include "Items.h"
#include "CoreDataContainer.h"
#include "ChunkFile.h"

class RegSegmentFileOperator;

typedef CoreDataContainer<RegSimpleItem, 64> LoadedItemsContainer;

// Since strings are used in the names of everything we expect them to be very common
typedef CoreDataContainer<RegString, 128> LoadedStringsContainer;
typedef CoreDataContainer<RegGroup, 16> LoadedGroupsContainer;
typedef CoreDataContainer<RegNonStringArray, 8> LoadedArraysContainer;

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
	void TrackChange(DataPosition pos);

	void Save();

	RegSegment();
};

namespace SegmentHandler
{
	RegSegment* GetOrLoadSegment(std::string& name);
	RegSegment* LoadSegment(std::string& name);

	// NOTE: This is expected to be running on just one thread at any given time.
	void SaveAllSegments();
}