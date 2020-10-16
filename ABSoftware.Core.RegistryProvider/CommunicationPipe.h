#pragma once
#include "General.h"
#include <string>
#include <vector>

// Handles communication to and from a client.
class CommunicationPipe {
public:
	std::string TextBuffer;

	CommunicationPipe(bool& successful, const wchar_t* pipeName);
	~CommunicationPipe();

	bool WaitForConnection();
	void WaitForDisconnection();

	bool Read(void* output, int toWrite);
	bool Write(void* input, int size);

	bool ReadStringToBuffer();
	bool WriteString(char* text, int textSize);

private:
#ifdef WINDOWS
	HANDLE _pipeHandle;
#endif
};