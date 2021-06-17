#include "pch.h"
#include "../ABSoftware.SystemState/ChunkFile.h"
#include "CppUnitTest.h"
#include "TestHelpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SystemStateTests
{
	TEST_CLASS(ChunkFileTests)
	{
	public:
		void WriteExpectedFileHeader(FileStream& stream, uint32_t fileInfoChunk, uint32_t rootInfoChunk, uint32_t freeSpaces)
		{
			stream.WriteInt32(1);
			stream.WriteInt32(fileInfoChunk);
			stream.WriteInt32(rootInfoChunk);
			stream.WriteInt32(freeSpaces);
		}

		void WriteExpectedFileInfoObject(FileStream& stream, uint32_t numberOfChunks, uint8_t* expectedBytes, bool fill)
		{
			stream.WriteInt32(numberOfChunks);

			int byteCount = (numberOfChunks / 8) + ((numberOfChunks % 8 > 0) ? 1 : 0);
			for (int i = 0; i < byteCount; i++) stream.WriteByte(expectedBytes[i]);

			// Fill the rest of the bytes with zero.
			if (fill)
			{
				int remaining = 60 - byteCount;
				for (int i = 0; i < remaining; i++) stream.WriteByte(0);
			}
		}

		void WriteExpectedRootObject(FileStream& stream, uint8_t rootObject, bool fill)
		{
			stream.WriteByte(rootObject);

			if (fill)
				for (int i = 0; i < 63; i++)
					stream.WriteByte(0);
		}

		TEST_METHOD(CreateChunkFile)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.WriteByte(2);
			file.Close();

			// See if the file contains what we're expecting.
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 0, 1, 0);

			uint8_t bytes[1] = { 0 };
			WriteExpectedFileInfoObject(stream, 2, bytes, true);
			WriteExpectedRootObject(stream, 2, false);

			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(CreateObject_Single_End)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();
			file.WriteByte(2);

			uint32_t pos = file.CreateObject(1);
			Assert::AreEqual((uint32_t)2, pos);

			file.JumpToChunk(pos);
			file.WriteByte(4);

			file.Flush();
			file.Close();

			// See if the file contains what we're expecting.
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 0, 1, 0);

			uint8_t bytes[1] = { 0 };
			WriteExpectedFileInfoObject(stream, 3, bytes, true);
			WriteExpectedRootObject(stream, 2, true);

			stream.WriteByte(4);
			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(DeleteObject_Single_End)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			// Create our object.
			uint32_t pos = file.CreateObject(1);

			// Delete the object
			file.DeleteObject(pos, 1);
			file.Flush();
			file.Close();

			// See if the file contains what we're expecting.
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 0, 1, 1);

			uint8_t bytes[1] = { 32 };
			WriteExpectedFileInfoObject(stream, 3, bytes, false);

			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(DeleteObject_Multiple_End)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			// Create our object.
			uint32_t pos = file.CreateObject(3);

			// Delete the object
			file.DeleteObject(pos, 3);
			file.Flush();
			file.Close();

			// See if the file contains what we're expecting.
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 0, 1, 3);

			uint8_t bytes[1] = { 56 };
			WriteExpectedFileInfoObject(stream, 5, bytes, false);

			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(CreateObject_Single_Middle)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			// Create space in the middle.
			file.CreateObject(1);
			file.CreateObject(1);
			file.DeleteObject(2, 1);

			// See if the object will fill that space.
			uint32_t pos = file.CreateObject(1);
			Assert::AreEqual((uint32_t)2, pos);

			file.Flush();
			file.Close();
		}

		TEST_METHOD(CreateObject_Multiple_End)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			uint32_t pos = file.CreateObject(2);
			Assert::AreEqual((uint32_t)2, pos);

			file.Flush();
			file.Close();
		}

		TEST_METHOD(CreateObject_Multiple_End_AcrossBytes)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			uint32_t pos = file.CreateObject(20);
			Assert::AreEqual((uint32_t)2, pos);

			file.Flush();
			file.Close();
		}

		TEST_METHOD(DeleteObject_Multiple_End_AcrossBytes)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			uint32_t pos = file.CreateObject(20);
			file.DeleteObject(pos, 20);

			file.Flush();
			file.Close();

			// See if the file contains what we're expecting.
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 0, 1, 20);

			uint8_t bytes[3] = { 63, 255, 252 };
			WriteExpectedFileInfoObject(stream, 22, bytes, false);

			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(CreateObject_Multiple_Middle)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			// Create space in the middle.
			file.CreateObject(2);
			file.CreateObject(1);
			file.DeleteObject(2, 2);

			uint32_t pos = file.CreateObject(2);
			Assert::AreEqual((uint32_t)2, pos);

			file.Flush();
			file.Close();
		}

		TEST_METHOD(CreateObject_Single_Middle_LotsOfBytes)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			// Create a huge chunk info
			// 8 bytes, 4 bytes, 2 bytes
			file.CreateObject(112);
			file.CreateObject(1);
			file.DeleteObject(114, 1);

			uint32_t pos = file.CreateObject(1);
			Assert::AreEqual((uint32_t)114, pos);

			file.Flush();
			file.Close();
		}

		TEST_METHOD(ResizeObject_Grow_End_SamePosition)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			uint32_t pos = file.CreateObject(1);
			bool moved = file.ResizeObject(&pos, 1, 2);
			Assert::AreEqual(false, moved);
			Assert::AreEqual((uint32_t)2, pos);

			file.Flush();
			file.Close();
		}

		TEST_METHOD(ResizeObject_Grow_Middle_SamePosition)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			file.CreateObject(1);
			file.CreateObject(1);
			file.DeleteObject(3, 1);

			uint32_t pos = 2;
			bool moved = file.ResizeObject(&pos, 1, 2);
			Assert::AreEqual(false, moved);
			Assert::AreEqual((uint32_t)2, pos);

			file.Flush();
			file.Close();
		}

		TEST_METHOD(ResizeObject_Grow_Middle_DifferentPosition)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			file.CreateObject(1);
			file.CreateObject(1);

			uint32_t pos = 2;
			bool moved = file.ResizeObject(&pos, 1, 2);
			Assert::AreEqual(true, moved);
			Assert::AreEqual((uint32_t)4, pos);

			// Make sure the first space became free.
			Assert::AreEqual((uint32_t)2, file.CreateObject(1));

			file.Flush();
			file.Close();
		}

		TEST_METHOD(ResizeObject_Shrink)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			file.CreateObject(3);
			file.CreateObject(1);

			uint32_t pos = 2;
			bool moved = file.ResizeObject(&pos, 3, 2);
			Assert::AreEqual(false, moved);
			Assert::AreEqual((uint32_t)2, pos);

			// Make sure new objects go in the right places.
			Assert::AreEqual((uint32_t)4, file.CreateObject(1));
			Assert::AreEqual((uint32_t)6, file.CreateObject(1));

			file.Flush();
			file.Close();
		}

		TEST_METHOD(Flush_DifferentRootChunkPos)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			file.CreateObject(1);
			file.ResizeObject(&file.RootObjectChunk, 1, 2);

			file.Flush();
			file.Close();

			// Check output
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 0, 3, 1);

			uint8_t bytes[1] = { 64 };
			WriteExpectedFileInfoObject(stream, 5, bytes, false);

			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(Flush_GrowFileInfoChunk_WithoutSecondMove)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();
			file.WriteByte(4);

			file.CreateObject(2);
			file.CreateObject(513);

			// Leave room for the chunk file info to go in.
			file.DeleteObject(2, 2);

			file.Flush();
			file.Close();

			// Check output
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 2, 1, 1);

			// Write the garbage data left over from where the file info previously was.
			stream.WriteInt32(2);

			stream.JumpTo(16 + 64);
			WriteExpectedRootObject(stream, 4, true);

			uint8_t bytes[65] = { 0 };
			bytes[0] = 128;

			WriteExpectedFileInfoObject(stream, 517, bytes, false);

			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(Flush_GrowFileInfoChunk_WithSecondMove)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();
			file.WriteByte(4);

			file.CreateObject(1022);

			file.Flush();
			file.Close();

			// Check output
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 1024, 1, 1);

			// Write the garbage data left over from where the file info previously was.
			stream.WriteInt32(2);

			stream.JumpTo(16 + 64);
			WriteExpectedRootObject(stream, 4, true);

			uint8_t bytes[129] = { 0 };
			bytes[0] = 128;

			stream.JumpTo(16 + 64 * 1024);
			WriteExpectedFileInfoObject(stream, 1027, bytes, false);

			stream.Close();
			AssertFileData();
		}

		TEST_METHOD(LoadFile)
		{
			CheckEndianness();

			// Create a new file.
			ChunkFile file;
			file.Create("actual.dat");
			file.PrepareForChanges();

			file.CreateObject(1);
			file.CreateObject(1);

			uint32_t pos = 2;
			file.ResizeObject(&pos, 1, 2);

			file.Flush();
			file.Close();

			// Load the file and make a change.
			ChunkFile loadedFile;
			file.Open("actual.dat");
			file.PrepareForChanges();

			Assert::AreEqual((uint32_t)2, file.CreateObject(1));
			file.DeleteObject(3, 1);

			file.Flush();
			file.Close();

			// Check output
			FileStream stream;
			stream.Create("expected.dat");

			WriteExpectedFileHeader(stream, 0, 1, 1);

			uint8_t bytes[1] = { 16 };
			WriteExpectedFileInfoObject(stream, 6, bytes, false);

			stream.Close();
			AssertFileData();
		}
	};
}