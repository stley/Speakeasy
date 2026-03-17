#include <iostream>
#include <thread>
#include <string>
#include <queue>
#include <mutex>
#include <rakChat.h>
#include "voice.hpp"
#include "device.hpp"
#include <atomic>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <Kbhit.h>
    #define _kbhit kbhit
    #define _getch getch
#endif

using namespace RakNet;


struct ConnectionConfig
{
    std::string serverIp;
    unsigned short serverPort = 65535;
    std::string userName;
};

enum MessageTypes : uint8_t
{
    SPK_MESSAGE_UNDEFINED = 0x00,
    SPK_SYSTEM_MESSAGE,
    SPK_CHAT_MESSAGE
};

struct ChatMessage
{
    uint8_t message_type = SPK_MESSAGE_UNDEFINED;
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
    SystemAddress serverAddress = UNASSIGNED_SYSTEM_ADDRESS;

    //Voice
    SpeakeasyEngine* voiceEngine = nullptr;

    void ClientThread();
    
    void ProcessSlashCommand(const std::string& cmdtext);

public:
    RakChatClient();
    ~RakChatClient();
    void ClientMain();
};