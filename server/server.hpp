#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <atomic>

#define RKC_MAX_CLIENTS (10)

#include <rakChat.h>
#include "RakSleep.h"

using namespace RakNet;


struct RakChatUser
{
    std::string Name;
    RakNet::SystemAddress userAddr;
    RakNet::RakNetGUID userGUID;
};



class RakChatServer
{
private:
    RakPeerInterface* peer;
    SocketDescriptor sd;
    std::thread workerThread;
    std::atomic<bool> isServerRunning = false;
    RakNet::Packet *packet;

    std::map<RakNet::RakNetGUID, RakChatUser> connectionList_;

    void MainThread();

    void HandlePacket(Packet *data);

    bool isNameAvailable(const char* name);

    bool isGuidRegistered(RakNetGUID guid_);

public:

    RakChatServer();
    ~RakChatServer();
};