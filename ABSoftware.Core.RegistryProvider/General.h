#pragma once
#include <string>
#include <filesystem>

extern std::filesystem::path ABSoftwareDirectory;
extern std::filesystem::path SegmentsDirectory;

// Enable for Windows, disable for Linux.
#define WINDOWS

#ifdef WINDOWS
#include <Windows.h>
#include <ShlObj.h>

#else
#endif