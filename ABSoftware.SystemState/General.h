#pragma once
#include <string>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <iostream>
#include <fstream>
#include <queue>
#include <variant>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "Helpers.h"

// Everything here is handled in "Main.cpp".
extern std::filesystem::path RegistryDirectory;
extern std::filesystem::path SegmentsDirectory;
extern bool IsLittleEndianSystem;

void CheckEndianness();
void Crash(std::string errorMsg);

template<typename T>
class SizedArray
{
public:
	int Size;
	T* Array;

	SizedArray()
	{
		Size = 32;
		Array = new T[Size];
	}

	SizedArray(int size)
	{
		Size = size;
		Array = new T[size];
	}

	~SizedArray()
	{
		delete[] Array;
	}
};

//class InPlaceString
//{
//private:
//	char* _data;
//
//public:
//	void Initialize(uint16_t size)
//	{
//		_data = new char[size + 2];
//		*((uint16_t*)_data) = size;
//	}
//
//	void Destroy()
//	{
//		delete[] _data;
//	}
//
//	char* GetDataAndSize()
//	{
//		return _data;
//	}
//
//	char* GetData()
//	{
//		return _data + 2;
//	}
//
//	uint16_t GetSize()
//	{
//		return *(uint16_t*)_data;
//	}
//};

// Enable for Windows, disable for Linux.
#define WINDOWS

#ifdef WINDOWS
#include <Windows.h>
#include <ShlObj.h>

#else
#endif