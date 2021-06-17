#include "FixedPositionVector.h"

int FixedPositionVector::Add(T item)
{
	if (NoOfFreeSpaces)
	{
		NoOfFreeSpaces--;

		// Find a free spot.
		size_t len = Data.size();
		for (size_t i = FreeSpaceStart; i < len; i++)
			if (Data[i] == nullptr)
			{
				Data[i] = item;
				FreeSpaceStart = i;
				return i;
			}
	}
	else
	{
		Data.push_back(item);
		return Data.size() - 1;
	}

	return -1;
}