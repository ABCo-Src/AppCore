#include <wchar.h>
#include "General.h"
#include "CommunicationPipe.h"

// Windows implementation
#ifdef WINDOWS
wchar_t* ConcatPipeLiteral(const wchar_t* with)
{
    int secondSize = wcslen(with);
    wchar_t* res = new wchar_t[secondSize + 10];

    memcpy(res, L"\\\\.\\pipe\\", 9 * sizeof(wchar_t));
    memcpy(res, with, secondSize * sizeof(wchar_t));

    return res;
}

CommunicationPipe::CommunicationPipe(bool& successful, const wchar_t* pipeName) 
{
    // Add "\\.\pipe\" before the pipe's name.
    wchar_t* withPipeIdentifier = ConcatPipeLiteral(pipeName);

    // Setup the pipe.
	_pipeHandle = CreateNamedPipe(
        withPipeIdentifier,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE,
        PIPE_UNLIMITED_INSTANCES,
        0,
        0,
        0,
        NULL);

    successful = _pipeHandle != INVALID_HANDLE_VALUE;
    delete[] withPipeIdentifier;
}

CommunicationPipe::~CommunicationPipe() 
{
    if (_pipeHandle != INVALID_HANDLE_VALUE) 
    {
        CloseHandle(_pipeHandle);
    }
}

bool CommunicationPipe::WaitForConnection() 
{
    return ConnectNamedPipe(_pipeHandle, NULL);
}

void CommunicationPipe::WaitForDisconnection()
{
    void* buffer;
    Read(&buffer, 1);
}

bool CommunicationPipe::Read(void* output, int toRead) {
    DWORD actualRead;
    return ReadFile(_pipeHandle, output, toRead, &actualRead, NULL);
}

bool CommunicationPipe::Write(void* input, int toWrite) {
    DWORD actualRead;
    return WriteFile(_pipeHandle, input, toWrite, &actualRead, NULL);
}
#endif

bool CommunicationPipe::ReadStringToBuffer()
{ 
    char size;
    if (!Read(&size, 1)) return false;

    TextBuffer.resize(size + 1);
    return Read(TextBuffer.data(), size);

    // Null-terminate the buffer.
    TextBuffer[size] = 0;
}

bool CommunicationPipe::WriteString(char* text, int textSize)
{
    return Write(text, textSize);
}