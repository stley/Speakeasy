#include <iostream>
#include <thread>
#include <string>
#include <queue>
#include <mutex>
#include <conio.h>
#include <rakChat.h>
#include "voice.hpp"
#include "device.hpp"
#include <atomic>

using namespace RakNet;


struct ConnectionConfig
{
    std::string serverIp;
    unsigned short serverPort = 65535;
    std::string userName;
};

struct ChatMessage
{
    std::string messageAuthor;
    std::string messageContent;
};




class RakChatClient
{
private:
    ConnectionConfig config_;
    RakNet::RakPeerInterface *peer;
    SocketDescriptor sd;
    RakNet::Packet* packet;
    std::thread workerThread;
    std::atomic<bool> running_ = false;
    std::atomic<bool> connected_ = false;
    std::queue<ChatMessage> MessageQueue;
    std::mutex queueMutex;

    //Voice
    SpeakeasyEngine* voiceEngine;
    AudioDevice* portDevice;

    void ClientThread();
    
    void ProcessSlashCommand(const char* input);

public:
    RakChatClient();
    ~RakChatClient();
    void ClientMain();
    RakPeerInterface* GetPeer() { return peer; }
};