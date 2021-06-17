#pragma once
#include <fstream>

class FileStream
{
private:
	std::fstream _fileData;

	template<typename T, int TSize>
	T DoRead();

	template<typename T, std::streamsize TSize>
	void DoWrite(T item);

public:
	virtual bool Create(const std::filesystem::path& path);
	virtual bool Open(const std::filesystem::path& path);
	void Close();

	uint32_t GetCurrentPos();
	void JumpTo(uint32_t pos);

	uint8_t ReadByte();
	void ReadBytes(char* dest, int size);
	uint16_t ReadInt16();
	uint32_t ReadInt32();
	int32_t ReadSignedInt32();
	uint64_t ReadInt64();
	int64_t ReadSignedInt64();
	float ReadSingle();
	double ReadDouble();

	void WriteByte(uint8_t data);
	void WriteBytes(char* src, int size);
	void WriteInt16(uint16_t data);
	void WriteInt32(uint32_t data);
	void WriteInt64(uint64_t data);
	void WriteSingle(float data);
	void WriteDouble(double data);

	~FileStream() { Close(); }
};