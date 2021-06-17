#include "General.h"
#include "Helpers.h"
#include <cstdlib>

bool FileStream::Create(const std::filesystem::path& path)
{
    _fileData.open(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    return true;
}

bool FileStream::Open(const std::filesystem::path& path)
{
    _fileData.open(path, std::ios::in | std::ios::out | std::ios::binary);
    return _fileData.is_open();
}

void FileStream::Close()
{
    _fileData.flush();
    _fileData.close();
}

uint32_t FileStream::GetCurrentPos()
{
    return (uint32_t)_fileData.tellg();
}

void FileStream::JumpTo(uint32_t pos)
{
    _fileData.seekg(pos);
}

template<typename T, int TSize>
T FileStream::DoRead()
{
    T res = 0;
    _fileData.read((char*)&res, TSize);

    if (TSize > 1 && !IsLittleEndianSystem)
    {
        T revRes = 0;

        // Reverse it
        char* destPos = (char*)&revRes;
        char* srcPos = ((char*)&res) + TSize;

        for (int i = 0; i < TSize; i++)
            *destPos++ = *--srcPos;

        return revRes;
    }
    return res;
}

uint8_t FileStream::ReadByte()
{
    return DoRead<uint8_t, 1>();
}

void FileStream::ReadBytes(char* dest, int size)
{
    _fileData.read(dest, size);
}

uint16_t FileStream::ReadInt16()
{
    return DoRead<uint16_t, 2>();
}

uint32_t FileStream::ReadInt32()
{
    return DoRead<uint32_t, 4>();
}

int32_t FileStream::ReadSignedInt32()
{
    return DoRead<int32_t, 4>();
}

uint64_t FileStream::ReadInt64()
{
    return DoRead<int64_t, 8>();
}

int64_t FileStream::ReadSignedInt64()
{
    return DoRead<long, 4>();
}

float FileStream::ReadSingle()
{
    return DoRead<float, 4>();
}

double FileStream::ReadDouble()
{
    return DoRead<double, 8>();
}

template<typename T, std::streamsize TSize>
void FileStream::DoWrite(T item)
{
    if (TSize > 1 && !IsLittleEndianSystem)
    {
        T revRes = 0;

        char* destPos = (char*)&revRes;
        char* srcPos = ((char*)&item) + TSize;

        for (int i = 0; i < TSize; i++)
            *destPos++ = *--srcPos;

        _fileData.write((char*)&revRes, TSize);
    }
    else _fileData.write((char*)&item, TSize);
}

void FileStream::WriteByte(uint8_t src)
{
    DoWrite<uint8_t, 1>(src);
}

void FileStream::WriteBytes(char* src, int size)
{
    _fileData.write(src, size);
}

void FileStream::WriteInt16(uint16_t val)
{
    DoWrite<uint16_t, 2>(val);
}

void FileStream::WriteInt32(uint32_t val)
{
    DoWrite<uint32_t, 4>(val);
}

void FileStream::WriteInt64(uint64_t val)
{
    DoWrite<uint64_t, 8>(val);
}

void FileStream::WriteSingle(float val)
{
    DoWrite<float, 4>(val);
}

void FileStream::WriteDouble(double val)
{
    DoWrite<double, 8>(val);
}