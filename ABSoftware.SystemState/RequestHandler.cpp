//#include "CommunicationPipe.h"
#include "General.h"
#include "RequestHandler.h"

// REQUEST CODES:
// xxyzzzzz - x = Position (0), Relative Path (1) or Full Path (2), y = Directory (0) or File (1), z = Operation Type
// x and y may not always be relevant to a certain operation, in which case they MUST be kept as "0".
//
// When accessing an ITEM: The group the item is contained in must be provided
//
// Operation Type:
//	- Prepare for rapid accesses (0)
//	- Finish rapid accesses (1)
//	- Get (2)
//	- Set (3)
//	- Add (4)
//	- Remove (5)
//  - List (6)
//	- Start notify (7) - GROUP ONLY
//	- Stop notify (8) - GROUP ONLY
//
enum class OperationType
{
	PrepareRapidAccess,
	FinishRapidAccess,
	GetItem,
	SetItem,
	AddItem,
	RemoveItem,
	ListGroups,
	ListItems,
	StartNotify,
	StopNotify
};

//void StartThread();

//void RequestHandler::HandleRequest(CommunicationPipe* pipe, uint8_t code)
//{
//	// If it's a fast operation, it will be executed right here without the overhead of thread creation.
//	// If it's a slow operation (like parsing a path), it will be executed on a seperate thread to not disrupt other operations.
//	switch ((OperationType)code)
//	{
//	// Rapid access technically causes all the operations of a given pipe to all be handled on their own thread.
//	case OperationType::PrepareRapidAccess:
//		StartThread(HandleRapidPipeAsync);
//		break;
//	}
//	
//}

//template<typename Fn>
//void StartThread(Fn f, CommunicationPipe* pipe, uint8_t code)
//{
//	// Start the thread.
//	std::thread t(f, pipe, code);
//	t.detach();
//
//	// Mark the pipe as being handled asynchornously so we don't start waiting for it.
//	pipe->HandledAsync = true;
//}
//
//void HandleRapidPipeAsync()
//{
//
//}