#include <iostream>
#include <filesystem>
#include "General.h"
#include "CommunicationPipe.h"
#include "RequestHandler.h"

//#ifdef WINDOWS
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
//#endif

void HandleConfigDirectory()
{
    wchar_t* appDataLoc;
#ifdef WINDOWS
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, NULL, NULL, &appDataLoc) != S_OK) throw "Unable to find the 'Application Data' directory.";
#else
#endif

    ABSoftwareDirectory = std::filesystem::path(appDataLoc).concat(L"\\ABSoftware\\");
    SegmentsDirectory = std::filesystem::path(ABSoftwareDirectory).concat(std::wstring(L"\\ABSoftware\\RegistrySegments\\"));

    // If it doesn't exist, we'll create the directory, and then the client will take it from there when it realizes we can't load up the right segment.
    if (!std::filesystem::is_directory(ABSoftwareDirectory))
        std::filesystem::create_directory(ABSoftwareDirectory);
    if (!std::filesystem::is_directory(SegmentsDirectory))
        std::filesystem::create_directory(SegmentsDirectory);

#ifdef WINDOWS
    CoTaskMemFree(appDataLoc);
#endif
    //stat();
}


void HandleIncomingConnections()
{
    bool successful;
    CommunicationPipe pipe(successful, L"ABSOFTWARE_REGISTRY_IncomingConnections");

    if (!successful) throw "Unable to start up ABSoftware registry!";

    while (true)
    {
        if (!pipe.WaitForConnection()) throw "Unable to start up ABSoftware registry!";

        RequestHandler::StartHandler();

        pipe.WaitForDisconnection();
    }
}

int main()
{
    // 
    HandleIncomingConnections();
    return 0;
}