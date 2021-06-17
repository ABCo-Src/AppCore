#pragma once

#include "General.h"
#include "CoreDataContainer.h"

enum class RegItemType : char
{
	Integer = 1,
	Decimal = 2,
	BlGroup = 3,

	// Used by arrays:
	String = 4
};

// Represents a simple item stored inline in a group
class RegSimpleItem
{
public:
	DataPosition Name;
	RegItemType Type = (RegItemType)0;

	union
	{
		int64_t Integer = 0;
		double Decimal;
	};
};

class RegObject
{
public:
	DataPosition Name;
	DataPosition Parent;

	bool IsLoaded = false;
	bool HasChanged = false;

	bool IsParentArray = false; // (For strings)

	uint32_t CurrentFileChunk = 0;
	uint32_t CurrentFileChunkLength = 0;

	// Shared for reads or modifications, not shared for addition or removal.
	std::shared_mutex ModifyLock;
};

class RegArray : public RegObject
{
public:
	RegItemType ItemType = (RegItemType)0;
	std::vector<DataPosition> Items;
};

class RegString : public RegObject
{
public:
	std::vector<char> Data;
};

// Represents a group of items.
class RegGroup : public RegObject
{
public:
	std::vector<DataPosition> SubItems;
	std::vector<DataPosition> SubGroups;
	std::vector<DataPosition> SubStrings;
	std::vector<DataPosition> SubArrays;
};