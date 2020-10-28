#pragma once
#include <vector>
#include "RegSegment.h"

namespace SegmentHandler
{
	RegSegment* GetOrLoadSegment(std::string& name);
	RegItem* GetItemByPath();
	RegSegment* SaveData();
}