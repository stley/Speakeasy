#include "Client.hpp"
class ExtendedClient : public RakChatClient
{
protected:
    virtual void ClientThread() override;
    virtual void ProcessSlashCommand(const std::string& cmdtext) override;

    void (*printHandler)(const char*) = nullptr;

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
            MessageQueue.push(out);
        }
        consoleBufUpdated = true;
    }

    std::atomic<bool> consoleBufUpdated {false};
    void ClientMain() override { };
public:
    
    
    ExtendedClient();
    ~ExtendedClient();
    void ClientConfigure(const char* ip, unsigned short port, const char* username);
    void ClientConnect();
    
    const char* FetchBuffer()
    {
        static std::queue<ChatMessage> localbuf;
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
            localbuf.pop();
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
    void SendMessage(const char* message);
    void Mute()
    {
        if (voiceEngine)
            voiceEngine->GetDevice()->Mute();
    }
};

