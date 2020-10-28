#include <unordered_map>
#include "General.h"
#include "SegmentHandler.h"

static std::mutex LoadingLock;
static std::atomic<int> CurrentNumberOfAccesses;
static std::vector<RegSegment*> LoadedSegments;

// SEGMENT LOADING:
// A segment file starts with a one-byte number, currently this should just be "1".
// Then, there are two 32-bit integers: Total number of items and total number of groups. These are used to prepare the maps for fast loading.
// After this comes a list of all the items directly in the root of the segment, all the groups are left "unloaded", and none of their sub-items are processed.
// The groups in this list are structured like this: | GROUP NAME | DATA POSITION |, where the data position is what exact position in the file the data is in. This data is loaded into the "KnownGroupsMap".
// At "Data Position", there is a list just like the root list, with all the items that are directly in that group.
// Then, if we ask for one of the groups, or one of the sub-items of a group, that group needs to be loaded up now, and in that case it has to manually parse the path to ensure all the groups involved are loaded.

void AddItemToGroups()
{
	LoadedGroupsFreeStart
}

RegGroupLookup* ParseList(std::ifstream& data, RegSegment& currentSegment, RegGroupLookup& currentGroup, std::string& currentPath)
{
	// Read the number of groups and number of items.
	char noOfGroups = 0;
	char noOfItems = 0;
	char maxPathSize = 0;

	data.read(&noOfItems, 1);
	data.read(&noOfGroups, 1);
	data.read(&maxPathSize, 1);

	RegGroup* newGroup = new RegGroup();
	newGroup->SubGroups.reserve(noOfGroups);
	newGroup->SubItems.reserve(noOfItems);

	int initialLength = currentPath.length();
	char* rootPathEnd = currentPath.data() + initialLength;

	for (int i = 0; i < noOfGroups; i++)
	{
		// Read the name of the group onto the path.
		char len = 0;
		data.read(&len, 1);
		data.read(rootPathEnd, len);

		int dataPos = 0;
		data.read((char*)&dataPos, len);

		newGroup->SubGroups.push_back(len);
		currentSegment.KnownGroupsMap.emplace(currentPath, RegGroupLookup(dataPos));
	}

	currentSegment.

	currentGroup.LoadedPosition = 

	// Size the string back down to what it was initially, so it can be re-used without any problems.
	currentPath.resize(initialLength);
}

RegSegment* ParseSegmentData(std::ifstream& data)
{
	RegSegment* res = new RegSegment();

	// The first byte should be "1", else fail.
	char first;
	if (!data.get(first) || first == 1) return nullptr;

	int noOfItems = 0;
	int noOfGroups = 0;

    data.read((char*)&noOfItems, 4);
	data.read((char*)&noOfGroups, 4);

	res->AllItems.reserve(noOfItems);
	res->KnownGroupsMap.reserve(noOfGroups);
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
	CurrentNumberOfAccesses++;

	auto loadedSegment = LoadedSegmentsMap.find(name);
	if (loadedSegment == LoadedSegmentsMap.end()) return LoadSegment(name);

	CurrentNumberOfAccesses--;
	return loadedSegment->second;
}