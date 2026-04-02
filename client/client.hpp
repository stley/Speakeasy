#include <iostream>
#include <thread>
#include <string>
#include <queue>
#include <mutex>
#include <rakChat.h>
#include "SpeakEngine.hpp"
#include "SoundDevice.hpp"
#include <atomic>




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

enum ClientState : uint8_t
{
    Init = 0x00,
    Connecting,
    Registering,
    RegistrationFailed,
    FailedConnection,
    Connected
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

    ClientState state_ = Init;

    //Voice
    SpeakeasyEngine* voiceEngine = nullptr;

    void ClientThread();
    
    void ProcessSlashCommand(const std::string& cmdtext);

public:
    RakChatClient();
    ~RakChatClient();
    ClientState GetClientState() { return state_; }
    void SetClientState(ClientState newState) { state_ = newState; }
    void ClientMain();
    void ClientConfigure(const char* ip, unsigned short port, const char* username);
    void ClientConnect();
    void Mute()
    {
        if (voiceEngine)
            voiceEngine->GetDevice()->Mute();
    }

    void SendMessage(const char* message);
};