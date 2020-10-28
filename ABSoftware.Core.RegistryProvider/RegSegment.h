#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include "RegItem.h"
#include "FixedPositionVector.h"

class RegSegment
{
	// NOTE: See the explanation in "Segment Handler" to understand the role of each item here.
public:
	std::mutex AccessGroup;
	std::unordered_map<std::string, int> ItemsMap;
	std::unordered_map<std::string, RegGroupLookup> KnownGroupsMap;

	FixedPositionVector<RegItem*> AllItems;
	FixedPositionVector<RegGroup*> LoadedGroups;
};