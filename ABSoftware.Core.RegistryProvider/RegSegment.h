#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include "RegItem.h"

class RegSegment : RegGroup
{
public:
	std::unordered_map<std::string, int> ItemsMap;
	std::unordered_map<std::string, int> GroupsMap;

	std::vector<RegItem*> AllItems;
	std::vector<RegGroup*> AllGroups;
};