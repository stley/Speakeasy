#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>

#define RKC_MAX_CLIENTS (10)

#include <rakChat.h>
#include "RakSleep.h"
#include "RakChatUser.hpp"

using namespace RakNet;

class RakChatServer
{
private:
    RakPeerInterface* peer;
    SocketDescriptor sd;
    std::thread workerThread;
    std::atomic<bool> isServerRunning = false;

    
    RakNet::Packet *packet;
    
    
    std::mutex listMutex;

    void MainThread();

    void HandlePacket(Packet *data);

    bool isNameAvailable(const char* name, size_t len);

    bool isGuidRegistered(RakNetGUID guid_);

    RakChatUserPool userPool;
public:

    RakChatServer();
    ~RakChatServer();
};