#pragma once
#include <vector>
#include <mutex>
#include "RegItem.h"

namespace RequestHandler
{
	void StartHandler();
	void DoWork(CommunicationPipe pipe);
}