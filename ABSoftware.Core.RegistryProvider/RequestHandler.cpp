#include "CommunicationPipe.h"
#include "RequestHandler.h"

void RequestHandler::StartHandler()
{
	bool successful;
	CommunicationPipe pipe(successful, L"ABSOFTWARE_REGISTRY_CommunicationLink");

	if (!successful) return;

	std::thread t(DoWork, pipe);
	t.detach();
}

void RequestHandler::DoWork(CommunicationPipe pipe)
{
	pipe.WaitForConnection();

	while (true)
	{
		char c = 0;
		if (!pipe.Read(&c, 1)) break;

		
	}
	
}