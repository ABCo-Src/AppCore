#include "General.h"
#include "Communication.h"
#include "RequestHandler.h"

//#ifdef WINDOWS
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
//#endif

std::filesystem::path RegistryDirectory;
std::filesystem::path SegmentsDirectory;
bool IsLittleEndianSystem;

constexpr bool platform_supported = 
    std::numeric_limits<float>::is_iec559
    && std::numeric_limits<double>::is_iec559
    && (sizeof(float) == 4)
    && (sizeof(double) == 8);

void HandleConfigDirectory()
{
    wchar_t* appDataLoc;
#ifdef WINDOWS
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, NULL, NULL, &appDataLoc) != S_OK) throw "Unable to find the 'Application Data' directory.";
#else
#endif

    // If it doesn't exist, we'll create the directory, and then the client will take it from there when it realizes we can't load up the right segment.
    RegistryDirectory = std::filesystem::path(appDataLoc) / L"ABSoftware";
    if (!std::filesystem::is_directory(RegistryDirectory)) std::filesystem::create_directory(RegistryDirectory);

    RegistryDirectory /= "Registry";
    if (!std::filesystem::is_directory(RegistryDirectory)) std::filesystem::create_directory(RegistryDirectory);

    SegmentsDirectory = RegistryDirectory / L"Segments";
    if (!std::filesystem::is_directory(SegmentsDirectory)) std::filesystem::create_directory(SegmentsDirectory);

#ifdef WINDOWS
    CoTaskMemFree(appDataLoc);
#endif
}

void HandleIncomingConnections()
{
    //CommunicationManager allConnections;

    //// Start up the incoming connection handler.
    //bool successful = false;
    //CommunicationPipe* pipe = new CommunicationPipe(successful, L"ABSOFTWARE_REGISTRY_IncomingConnections");

    //if (!successful) throw "Unable to start up ABSoftware registry!";

    //allConnections.AddConnection(pipe);

    //while (true)
    //{
    //    // Wait for a request.
    //    CommunicationPipe* pipe = allConnections.WaitForOne();

    //    RequestHandler::HandleRequest();

    //    pipe.WaitForDisconnection();
    //}
}

void CheckEndianness()
{
    int endian_test = 0x00000001;
    IsLittleEndianSystem = (*(char*)&endian_test == 0x01) ? true : false;
}

int main()
{
    CheckEndianness();
    HandleConfigDirectory();

    if (!platform_supported)
        Crash("The given platform does not support 64-bit iec559 decimals, which ABSoftware depends on.");

    HandleIncomingConnections();
    return 0;
}

// =============
// FILE STREAM IMPL
// =============


void Crash(std::string msg)
{
    std::filesystem::path p = std::filesystem::path(RegistryDirectory) / "LastError.txt";

    std::ofstream writer(RegistryDirectory, std::ios::out);
    writer.write(msg.c_str(), msg.size());
    throw "Something went wrong in the ABSoftware system state! Please check 'LastError.txt' for more information.";
}