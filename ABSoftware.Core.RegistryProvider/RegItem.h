#pragma once
#include <string>

const char REG_ITEM_TYPE_INTEGER = 1;
const char REG_ITEM_TYPE_DECIMAL = 2;
const char REG_ITEM_TYPE_TEXT = 3;
const char REG_ITEM_TYPE_BOOLEAN_GROUP = 4;

class RegItem
{
public:
	char Type;
};

class IntegerAndBlGroupRegItem : RegItem
{
public:
	int Data;
};

class DecimalRegItem : RegItem
{
public:
	double Data;
};

class StringRegItem : RegItem
{
public:
	std::string* Data;

	~StringRegItem()
	{
		delete Data;
	}
};

class RegGroup
{
public:
	std::vector<RegItem*> SubItems;
	std::vector<RegGroup*> SubGroups;
};