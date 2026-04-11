#include "Client.hpp"
#include <unordered_map>
#include "RPC4Plugin.h"

struct Channel
{
    uint16_t channelParent;
    std::string channelName;
};
struct User
{
    uint16_t channelId;
    std::string UserName;
};

class ExtendedClient : public RakChatClient
{
protected:
    virtual void ClientThread() override;
    virtual void ProcessSlashCommand(const std::string& cmdtext) override;

    std::deque<ChatMessage> MessageQueue;
    std::unordered_map<uint16_t, Channel> ChannelMap;
    std::unordered_map<uint16_t, User> PeerMap;
    


    RPC4 rpc4;

    

    std::atomic<bool> consoleBufUpdated {false};
    void ClientMain() override { };
    void rpcRegister();
public:

    std::mutex cMapMutex_;
    std::mutex pMapMutex_;
    
    ExtendedClient();
    ~ExtendedClient();
    void ClientConfigure(const char* ip, unsigned short port, const char* username);
    void ClientConnect();

    void ConsolePrint(const char* fmt, ...)
    {
        char buffer[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        ChatMessage out;
        out.message_type = SPK_SYSTEM_MESSAGE;
        out.messageContent = buffer;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if(MessageQueue.size() >= 500)
                MessageQueue.pop_front();
            MessageQueue.push_back(out);
        }
        consoleBufUpdated = true;
    }

    const char* FetchBuffer()
    {
        static std::deque<ChatMessage> localbuf;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            std::swap(localbuf, MessageQueue);
        }
        static std::string buffstring;
        while (!localbuf.empty())
        {
            ChatMessage m = localbuf.front();
            
            switch (m.message_type)
            {
                case SPK_CHAT_MESSAGE:
                {
                    buffstring += m.messageAuthor;
                    buffstring += ": ";
                    buffstring += m.messageContent;
                    buffstring += "\n";
                    break;
                }
                case SPK_SYSTEM_MESSAGE:
                {
                    buffstring += m.messageAuthor;
                    buffstring += " ";
                    buffstring += m.messageContent;
                    buffstring += "\n";
                    break;
                }
                default:
                    break;
            }  
            localbuf.pop_front();
        }
        return buffstring.data();
    }

    bool PollBuff()
    {
        bool ret = consoleBufUpdated;
        if(consoleBufUpdated)
            consoleBufUpdated = !consoleBufUpdated;
        return ret;
    }

    std::unordered_map<uint16_t, Channel>& GetChannels() { return ChannelMap; };
    std::unordered_map<uint16_t, User>& GetUsers() { return PeerMap; };
    void PushChannel(uint16_t channelId, uint16_t parentId, const char* channelname)
    {
        Channel inC;
        inC.channelParent = parentId;
        inC.channelName = channelname;    
        ChannelMap.emplace(channelId, inC);
    }
    void PushUser(uint16_t userid, uint16_t channelid, const char* username)
    {
        User inU;
        inU.channelId = channelid;
        inU.UserName = username;
        PeerMap.emplace(userid, inU);
    }
    void Dropuser(uint16_t peerId);
    void SendMessage(const char* message);
    void SendPacket(RakNet::BitStream* bs, PacketPriority priority, PacketReliability reliability, char orderingChannel)
    {
        peer->Send(bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverAddress, false);
    }
    bool Mute()
    {
        if (voiceEngine)
            return voiceEngine->GetDevice()->Mute();
        return false;
    }
};

