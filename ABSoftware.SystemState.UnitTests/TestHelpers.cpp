#pragma once
#include "pch.h"
#include "TestHelpers.h"
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

std::string ReadFileData(const std::string& name)
{
	std::ifstream reader;
	reader.open(name, std::ios::in | std::ios::binary);
	reader.ignore((std::numeric_limits<std::streamsize>::max)());

	std::string res((uint32_t)reader.gcount(), 0);

	reader.seekg(0);
	reader.read(res.data(), res.size());
	return res;
}

//void AssertStringsAreEqual(std::string& first, std::string& second)
//{
//	if (first.size() != second.size()) Assert::Fail(L"Assert against two strings failed, different sizes!");
//	
//	for (int i = 0; i < first.size(); i++)
//		if (first[i] != second[i])
//			Assert::Fail(L"Assert against two strings failed, didn't match! Pos: " + i);
//}

void AssertFileData()
{
	std::string expected = ReadFileData("expected.dat");
	std::string actual = ReadFileData("actual.dat");

	Assert::AreEqual(expected, actual);
	//AssertStringsAreEqual(expected, actual);
}