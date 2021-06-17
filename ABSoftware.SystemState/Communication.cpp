#include "General.h"
//#include <wchar.h>
//#include <mutex>
//#include "General.h"
//#include "Communication.h"
//
//namespace CommunicationHandler
//{
//    static std::vector<CommunicationPipe*> Pipes;
//
//#ifdef WINDOWS
//    //static Spinlock _eventsProtection;
//    static std::vector<HANDLE> _events;
//
//    // Sync only
//    void AddConnection(CommunicationPipe* pipe)
//    {
//        HANDLE event = CreateEvent(NULL, true, false, NULL);
//
//        // Add the event.
//        {
//            //std::lock_guard<Spinlock> eventLock(_eventsProtection);
//            _events.push_back(event);
//        }
//
//        Pipes.push_back(pipe);
//    }
//
//    // Sync only
//    WaitResult WaitForOne()
//    {
//        //DWORD index = WaitForMultipleObjects(_events.size(), _events.data(), FALSE, INFINITE) - WAIT_OBJECT_0;
//
//        //if (index < 0 || index > Pipes.size()) throw "The ABSoftware registry was unable to successfully perform a waiting operation. Waiting attempted to access data outside the bounds of current pipes.";
//
//        //HANDLE succeededOverlap = Pipes[index];
//        //bool success = GetOverlappedResult(p._pipe)
//
//        //CommunicationPipe* pipe = Pipes[index];
//        //char* code = 
//        //return WaitResult(pipe, pipe->);
//    }
//#endif
//
//}
//
//// CommunicationPipe:
//// Windows implementation - Pipe
//#ifdef WINDOWS
//wchar_t* ConcatPipeLiteral(const wchar_t* with)
//{
//    int secondSize = wcslen(with);
//    wchar_t* res = new wchar_t[(size_t)secondSize + 10];
//
//    memcpy(res, L"\\\\.\\pipe\\", 9 * sizeof(wchar_t));
//    memcpy(res + 9, with, secondSize * sizeof(wchar_t));
//    res[secondSize + 9] = 0;
//
//    return res;
//}
//
//CommunicationPipe::CommunicationPipe(bool& successful, const wchar_t* pipeName) 
//{
//    //TextBuffer = std::string(0, '\0');
//    //TextBuffer.reserve(255);
//
//    // Add "\\.\pipe\" before the pipe's name.
//    wchar_t* withPipeIdentifier = ConcatPipeLiteral(pipeName);
//
//    // Setup the pipe.
//	_pipeHandle = CreateNamedPipe(
//        withPipeIdentifier,
//        PIPE_ACCESS_DUPLEX,
//        PIPE_TYPE_BYTE,
//        PIPE_UNLIMITED_INSTANCES,
//        0,
//        0,
//        0,
//        NULL);
//
//    successful = _pipeHandle != INVALID_HANDLE_VALUE;
//    delete[] withPipeIdentifier;
//}
//
//CommunicationPipe::~CommunicationPipe() 
//{
//    if (_pipeHandle != INVALID_HANDLE_VALUE) 
//    {
//        CloseHandle(_pipeHandle);
//    }
//}
//
//void CommunicationPipe::WindowsSetWaitEvent(HANDLE handle)
//{
//    _overlapped.hEvent = handle;
//}
//
//bool CommunicationPipe::WaitForConnection() 
//{
//    ConnectNamedPipe(_pipeHandle, &_overlapped);
//
//    DWORD noTransferred;
//    return GetOverlappedResult(_pipeHandle, &_overlapped, &noTransferred, true);
//}
//
//void CommunicationPipe::WaitForDisconnection()
//{
//    uint8_t buf;
//    StartRead(&buf, 1);
//    FinishRead();
//}
//
//void CommunicationPipe::StartRead(char* output, int toRead)
//{
//    _requestedReadCount = toRead;
//    ReadFile(_pipeHandle, output, toRead, NULL, &_overlapped);
//}
//
//bool CommunicationPipe::FinishRead()
//{
//    DWORD bytesRead;
//    bool res = GetOverlappedResult(_pipeHandle, &_overlapped, &bytesRead, true);
//    if (bytesRead != _requestedReadCount) return false;
//
//    return res;
//}
//
//void CommunicationPipe::StartWrite(char* data, int toWrite)
//{
//    _requestedWriteCount = toWrite;
//    WriteFile(_pipeHandle, data, toWrite, NULL, &_overlapped);
//}
//
//bool CommunicationPipe::FinishWrite()
//{
//    DWORD bytesWritten;
//    bool res = GetOverlappedResult(_pipeHandle, &_overlapped, &bytesWritten, true);
//
//    if (bytesWritten != _requestedReadCount) return false;
//    return res;
//}
//#endif
//
//bool CommunicationPipe::ReadString()
//{
//    uint16_t size;
//    StartRead((char*)&size, 2);
//    if (!FinishRead()) return false;
//
//    CurrentTextBuffer.resize(size + 1);
//    StartRead(CurrentTextBuffer.data(), size);
//
//    return FinishRead();
//}
//
//bool CommunicationPipe::WriteString(char* text, uint8_t textSize)
//{
//    StartWrite(&textSize, 1);
//    if (!FinishWrite()) return false;
//
//    StartWrite(text, textSize);
//    return FinishRead();
//}