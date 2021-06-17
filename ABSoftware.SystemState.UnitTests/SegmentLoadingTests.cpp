#include "pch.h"
#include "TestHelpers.h"
#include "../ABSoftware.SystemState/Segment.h"
#include "../ABSoftware.SystemState/Saving.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SystemStateTests
{
	TEST_CLASS(SegmentLoadingTests)
	{
	public:
		RegSegment* CurrentSegment;
		RegSegmentFileOperator* CurrentSegmentFile;
		ChunkFile* File;

		void Initialize()
		{
			CheckEndianness();

			// Create a file and put an empty root group in it.
			{
				ChunkFile file;
				file.Create("expected.dat");
				file.WriteInt32(0);
				file.Flush();
				file.Close();
			}

			CurrentSegment = new RegSegment();
			CurrentSegmentFile = CurrentSegment->File;
			CurrentSegmentFile->Initialize("expected.dat");
			File = &CurrentSegmentFile->File;
		}

		TEST_METHOD(Open_EmptyRootGroup)
		{
			Initialize();

			Assert::AreEqual((uint32_t)0, (uint32_t)CurrentSegment->RootGroup);

			RegGroup& group = CurrentSegment->LoadedGroups.Get(CurrentSegment->RootGroup);
			Assert::AreEqual((size_t)0, group.SubItems.size());
			Assert::AreEqual((size_t)0, group.SubGroups.size());
			Assert::AreEqual((size_t)0, group.SubArrays.size());
		}

		TEST_METHOD(LoadString)
		{
			Initialize();

			// Create the string
			uint32_t stringChunk = File->CreateObject(1);
			File->JumpToChunk(stringChunk);
			File->WriteInt32(((int)RegItemType::Char << 24) | 3);
			File->WriteBytes("abC", 3);

			// Try to load it.
			DataPosition newArrayPos;
			RegArray& newArray = CurrentSegment->LoadedArrays.Emplace(newArrayPos);
			newArray.CurrentFileChunk = stringChunk;
			CurrentSegmentFile->LoadArray(newArray);

			Assert::AreEqual((uint32_t)0, (uint32_t)newArray->);

			RegGroup& group = CurrentSegment->LoadedGroups.Get(CurrentSegment->RootGroup);
			Assert::AreEqual((size_t)0, group.SubItems.size());
			Assert::AreEqual((size_t)0, group.SubGroups.size());
			Assert::AreEqual((size_t)0, group.SubArrays.size());
		}

		//TEST_METHOD(LoadSimpleItem_Integer)
		//{
		//	Initialize();

		//	// Create the simple item into a new chunk.
		//	uint32_t newChunk = File->CreateObject(1);
		//	File->JumpToChunk(newChunk);

		//	File->WriteInt32();
		//}

		//TEST_METHOD(LoadSimpleItem_Integer)
		//{
		//	// Create the data.
		//	{
		//		ChunkFile file;
		//	}

		//	Initialize();
		//	//CurrentSegmentFile

		//}
	};
}