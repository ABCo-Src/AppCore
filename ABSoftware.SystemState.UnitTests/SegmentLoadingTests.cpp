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

		void FillFile(char* contents, int contentsLength)
		{
			// Create a file and put an empty root group in it.			
			ChunkFile file;
			file.Create("expected.dat");
			file.WriteInt32(0);
			file.Flush();
			file.Close();
		}

		ChunkFile* GetFile()
		{
			ChunkFile* file = new ChunkFile();
			file->Create("expected.dat");
			return file;
		}

		void FinishFile(ChunkFile* file)
		{
			file->Flush();
			file->Close();
			delete file;
		}

		void Initialize()
		{
			CurrentSegment = new RegSegment();
			CurrentSegmentFile = CurrentSegment->File;
			CurrentSegmentFile->Initialize("expected.dat");
			File = &CurrentSegmentFile->File;
		}

		TEST_METHOD(Open_EmptyRootGroup)
		{
			Setup();

			// Create a file and put an empty root group in it.
			{
				ChunkFile* file = GetFile();
				file->WriteInt32(0);
				FinishFile(file);
			}

			Initialize();

			Assert::AreEqual((uint32_t)0, (uint32_t)CurrentSegment->RootGroup);

			RegGroup& group = CurrentSegment->LoadedGroups.Get(CurrentSegment->RootGroup);
			Assert::AreEqual((size_t)0, group.SubItems.size());
			Assert::AreEqual((size_t)0, group.SubGroups.size());
			Assert::AreEqual((size_t)0, group.SubStrings.size());
			Assert::AreEqual((size_t)0, group.SubArrays.size());
		}

		void WriteRootGroupWithSingleString()
		{
			ChunkFile* file = GetFile();

			uint32_t stringName = file->CreateObject(1);
			file->JumpToChunk(stringName);
			file->WriteInt32(4);
			file->WriteBytes("name", 4);

			uint32_t stringChunk = file->CreateObject(1);
			file->JumpToChunk(stringChunk);
			file->WriteInt32(3);
			file->WriteBytes("abc", 3);

			file->JumpToChunk(file->RootObjectChunk);
			file->WriteInt32(0x01000000);
			file->WriteInt32(stringName);
			file->WriteInt32(stringChunk);

			FinishFile(file);
		}

		TEST_METHOD(ReadSingleString_InGroup_IsUnloaded)
		{
			Setup();

			// Create a file and put the string "abc" with the name "name" in it.
			WriteRootGroupWithSingleString();
			Initialize();

			RegGroup& rootGroup = CurrentSegment->LoadedGroups.Get(CurrentSegment->RootGroup);
			RegString& string = CurrentSegment->LoadedStrings.Get(rootGroup.SubStrings[0]);
			Assert::IsFalse(string.IsLoaded);

			RegString& stringName = CurrentSegment->LoadedStrings.Get(rootGroup.SubStrings[0]);
			Assert::IsFalse(stringName.IsLoaded);
		}

		TEST_METHOD(LoadSingleString_InGroup)
		{
			Setup();

			// Create a file and put the string "abc" with the name "name" in it.
			WriteRootGroupWithSingleString();
			Initialize();

			RegGroup& rootGroup = CurrentSegment->LoadedGroups.Get(CurrentSegment->RootGroup);
			RegString& string = CurrentSegment->LoadedStrings.Get(rootGroup.SubStrings[0]);
			LoadStr(string);
			Assert::IsFalse(string.IsLoaded);

			RegString& stringName = CurrentSegment->LoadedStrings.Get(rootGroup.SubStrings[0]);
			Assert::IsFalse(stringName.IsLoaded);
		}

		TEST_METHOD(Open_SimpleItem)
		{
			Setup();

			// Create a file and put an empty root group in it.
			{
				ChunkFile* file = GetFile();
				file->WriteInt32(0);
				FinishFile(file);
			}

			Initialize();


			Assert::AreEqual((uint32_t)0, (uint32_t)CurrentSegment->RootGroup);

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