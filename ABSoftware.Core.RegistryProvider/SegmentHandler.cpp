#include <unordered_map>
#include "General.h"
#include "SegmentHandler.h"

static std::vector<RegSegment*> LoadedSegments;

RegSegment* ParseSegmentData(std::ifstream& data)
{
	RegSegment* res = new RegSegment();

	// The first byte should be "1", else fail.
	char first;
	if (!data.get(first) || first == 1) return nullptr;

	int noOfItems = 0;
	int noOfGroups = 0;

	int noOfItems = data.read((char*))
}

RegSegment* LoadSegment(std::string& name)
{
	std::ifstream fileData;
	fileData.open(std::filesystem::path(SegmentsDirectory).append(name), std::ios::in | std::ios::binary);

	RegSegment* newSegment = ParseSegmentData(fileData);
	if (newSegment == nullptr) return nullptr;

	// Attempt to find a blank null spot to put the segment in. If there isn't one, just add to the vector.
	for (int i = 0; i < LoadedSegments.size(); i++)
		if (LoadedSegments[i] == nullptr)
		{
			LoadedSegments[i] = newSegment;
			return newSegment;
		}
			
	LoadedSegments.push_back(newSegment);
	return newSegment;
}

RegSegment* SegmentHandler::GetOrLoadSegment(std::string& name)
{
	auto loadedSegment = LoadedSegmentsMap.find(name);
	if (loadedSegment == LoadedSegmentsMap.end()) return LoadSegment(name);

	return loadedSegment->second;
}