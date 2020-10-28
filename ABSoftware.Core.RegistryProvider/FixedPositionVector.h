#pragma once
#include <vector>
#include <stack>
#include <atomic>
#include <queue>
#include <mutex>

// Represents a "vector" that always keeps items in the same position, even if you remove from the middle. This behaves almost like a mini-heap.
template<typename T>
class FixedPositionVector
{
private:
	std::queue<int> _freeSpaces;

	int Add(T item)
	{
		if (_freeSpaces.empty())
		{
			Data.push_back(item);
			return Data.size() - 1;
		}
		else
		{
			int pos = _freeSpaces.pop();
			Data[pos] = item;
			return pos;
		}
	}

	void Remove(int pos)
	{
		_freeSpaces.push_back(pos);
	}
public:
	std::vector<T> Data;
};