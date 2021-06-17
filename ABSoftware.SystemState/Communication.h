//#pragma once
//
//#include "General.h"
//#include <string>
//#include <vector>
//
//// Handles communication to and from a client.
//class CommunicationPipe {
//public:
//	std::string CurrentTextBuffer;
//	bool HandledAsync;
//
//	CommunicationPipe(bool& successful, const wchar_t* pipeName);
//	~CommunicationPipe();
//
//#ifdef WINDOWS
//	void WindowsSetWaitEvent(HANDLE handle);
//#endif
//
//	bool WaitForConnection();
//	void WaitForDisconnection();
//
//	void StartRead(char* output, int toRead);
//	bool FinishRead();
//
//	void StartWrite(char* input, int size);
//	bool FinishWrite();
//
//	bool ReadString();
//	bool WriteString(char* text, uint8_t textSize);
//
//private:
//	int _requestedReadCount;
//	int _requestedWriteCount;
//#ifdef WINDOWS
//	OVERLAPPED _overlapped;
//	HANDLE _pipeHandle;
//#endif
//};
//
//struct WaitResult
//{
//	CommunicationPipe* Pipe;
//	uint8_t PipeIndex;
//	uint8_t Code;
//
//	WaitResult(CommunicationPipe* pipe, uint8_t code, uint8_t value)
//	{
//		Pipe = pipe;
//		Code = code;
//		PipeIndex = value;
//	}
//};
//
//namespace CommunicationManager
//{
//	void AddConnection(CommunicationPipe* pipe);
//	WaitResult WaitForOne();
//
//	void MarkAsAsyncConnection();
//	void MarkAsNonAsyncConnection();
//};