#include "client.hpp"


RakChatClient::RakChatClient()
{
    peer = RakNet::RakPeerInterface::GetInstance();
    peer->Startup(1, &sd, 1);
    workerThread = std::thread(&RakChatClient::ClientThread, this);
    printf("RakChat Client started.\n");
}
RakChatClient::~RakChatClient()
{
    running_ = false;
    connected_ = false;
    peer->Shutdown(300);
    if(workerThread.joinable())
    {
        workerThread.join();
    }
    
    RakNet::RakPeerInterface::DestroyInstance(peer);
}


void RakChatClient::ClientThread()
{
    //printf("Packet thread: It's on! :)\n");
    while (!running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    while(running_)
    {
        for (packet=peer->Receive(); packet; peer->DeallocatePacket(packet), packet=peer->Receive())
		{
			switch (packet->data[0])
			{
                case ID_REGISTER_ME:
                    {
                        BitStream bs = BitStream(packet->data, packet->length, false);
                        bs.IgnoreBytes(sizeof(RakNet::MessageID));
                        unsigned char result;
                        bs.Read(result);
                        if (result == 'Y')
                        {
                            printf("You registered as %s.", config_.userName.c_str());
                            printf(" Now you can send messages.\n");
                            this->connected_ = true;
                        }
                        else if (result == 'O')
                        {
                            printf("Registration failed because name is in use.\n");
                            config_.serverIp.clear();
                            config_.userName.clear();
                            config_.serverPort = 65535;
                            peer->Shutdown(0);
                            peer->Startup(1, &sd, 1);
                            this->ClientMain();
                        }
                        else
                        {
                            printf("Registration failed.");
                            config_.serverIp.clear();
                            config_.userName.clear();
                            config_.serverPort = 65535;
                            peer->Shutdown(0);
                            peer->Startup(1, &sd, 1);
                            this->ClientMain();    
                        }
                        
                    }
                        break;
                case ID_CONNECTION_REQUEST_ACCEPTED:
				    {
				    	printf("You are connected! Registering you...\n");
                        
                        BitStream bs = BitStream();
                        bs.Write((RakNet::MessageID)ID_REGISTER_ME);
                        bs.Write(config_.userName.c_str());
                        peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
				    }
				    break;

                case ID_CONNECTION_LOST:
				    printf("Connection lost.\n");
				    break;

                case ID_CHAT_MESSAGE:
                {
                    RakString rs_author;
                    RakString rs_msg;
                    BitStream bsIn = BitStream(packet->data, packet->length, false);
                    bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
                    bsIn.Read(rs_author);
                    bsIn.Read(rs_msg);
                    ChatMessage message;
                    message.messageAuthor = rs_author.C_String();
                    message.messageContent = rs_msg.C_String();

                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        MessageQueue.push(message);
                    }
                }
                    break;

                case ID_SYSTEM_MESSAGE:
                {
                    RakString rs_msg;
                    BitStream bsIn = BitStream(packet->data, packet->length, false);
                    bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
                    bsIn.Read(rs_msg);
                    ChatMessage message;
                    message.messageAuthor = "\033[36m~";
                    message.messageContent = rs_msg.C_String();
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        MessageQueue.push(message);
                    }
                }
                    break;
                    
                case ID_NO_FREE_INCOMING_CONNECTIONS:
    		        printf("The server is full.\n");
			        break;
            }
        }
    }
}

void RakChatClient::ProcessSlashCommand(const char* input)
{
    if(strcmp(input, "/myname") == 0)
    {
        std::cout << "You are registered as " << config_.userName << ".\n";
    }
    else if(strcmp(input, "/exit") == 0)
    {
        std::cout << "Disconnecting from server..." << "\n";
        running_ = false;
        connected_ = false;
        peer->Shutdown(300);
        //peer->Shutdown(0);
    }
}

void RakChatClient::ClientMain()
{
    while (config_.serverIp.empty())
    {
        printf("Input server IP Address: ");
        std::getline(std::cin, config_.serverIp);
    }

    while (true)
    {
        printf("Input server port: ");
        std::string portString;
        std::getline(std::cin, portString);
        if(portString.empty())
            continue;

        try
        {
            int port = std::stoi(portString);
            if (port >= 1 && port <= 65535)
            {
                config_.serverPort = static_cast<unsigned short>(port);
                break;
            } 
            else
            {
                std::cout << "Port must be between 1 and 65535.\n";
            }
        }
        catch (...)
        {
            std::cout << "Invalid port. Please enter a number.\n";
        }
    }

    while (config_.userName.empty())
    {
        printf("Please enter your username: ");
        std::getline(std::cin, config_.userName);
    }
    printf("Done! We will patch you up to %s:%d, %s...\n", config_.serverIp.c_str(), config_.serverPort, config_.userName.c_str());
    
    peer->Connect(config_.serverIp.c_str(), config_.serverPort, 0, 0);
    running_ = true;


    std::string writeBuffer;

    while (!connected_ && running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
    }
    std::cout << "> " << std::flush;
    while (connected_) 
    { 
    
        std::queue<ChatMessage> localQueue;

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            std::swap(localQueue, MessageQueue);
        }

        while (!localQueue.empty())
        {
            ChatMessage m = localQueue.front();
            std::cout << "\r\33[2K";
            localQueue.pop();
            std::cout << m.messageAuthor << ": " 
                << m.messageContent << "\n";
                std::cout << "> " << writeBuffer << std::flush;
            
        }
        
        if (_kbhit())
        {
            int ch = _getch();
            
            if(ch == 8)
            {
                if(!writeBuffer.empty())
                {
                    writeBuffer.pop_back();
                    std::cout << "\b \b";
                }

            }

            else if (ch == '\r')
            {
                if (writeBuffer.empty()) continue;
                if (writeBuffer[0] == '/')
                {
                    std::cout << "\n";
                    ProcessSlashCommand(writeBuffer.c_str());
                }
                else
                {
                    BitStream bsOut = BitStream();
                    bsOut.Write((RakNet::MessageID)ID_CHAT_MESSAGE);
                    //bsOut.Write(config_.userName.c_str());
                    bsOut.Write(writeBuffer.c_str());
                    peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);    
                }
                writeBuffer.clear();
                std::cout << "\r\33[2K> " << std::flush;
            }
            else
            {
                writeBuffer.push_back(ch);
                std::cout << static_cast<char>(ch);
            }

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}

int main()
{
    RakChatClient theClient;
    
    theClient.ClientMain();

    return 0;
}