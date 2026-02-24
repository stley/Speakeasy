#include "server.hpp"



RakChatServer::RakChatServer()
{
    peer = RakPeerInterface::GetInstance();
    this->sd = SocketDescriptor(60000, 0);
    peer->Startup(RKC_MAX_CLIENTS, &sd, 1);
    peer->SetMaximumIncomingConnections(RKC_MAX_CLIENTS);
    std::cout << "RakChat Server started on port 60000.\n";
    isServerRunning = true;
    workerThread = std::thread(&RakChatServer::MainThread, this);
}
RakChatServer::~RakChatServer()
{
    isServerRunning = false;
    if(workerThread.joinable())
    {
        workerThread.join();
    }
    RakNet::RakPeerInterface::DestroyInstance(peer);
}


bool RakChatServer::isNameAvailable(const char* name)
{
    for (const auto& [guid, user] : connectionList_)
    {
        if(strcmp(user.Name.c_str(), name) == 0)
            return false;
    }
    return true;
}

bool RakChatServer::isGuidRegistered(RakNetGUID guid_)
{
    for (const auto& [guid, user] : connectionList_)
    {
        if(user.userGUID == guid_)
            return true;
    }
    return false;
}


void RakChatServer::HandlePacket(Packet *packet)
{
    switch(packet->data[0])
    {
        case ID_REGISTER_ME:
        {
            BitStream reg_bs = BitStream(packet->data, packet->length, false);
            reg_bs.IgnoreBytes(sizeof(RakNet::MessageID));
            RakString name;
            reg_bs.Read(name);
            BitStream response = BitStream();
            if(!isNameAvailable(name.C_String()))
            {
                
                response.Write((RakNet::MessageID)ID_REGISTER_ME);
                response.Write((unsigned char)'O');
                peer->Send(&response, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
            }
            else
            {
                RakChatUser theUser;
                theUser.userAddr = packet->systemAddress;
                theUser.userGUID = packet->guid;
                theUser.Name = name.C_String();
                {
                    std::lock_guard<std::mutex> lock(listMutex);
                    connectionList_.emplace(packet->guid, theUser);
                }
                
                response.Write((RakNet::MessageID)ID_REGISTER_ME);
                response.Write((unsigned char)'Y');
                peer->Send(&response, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
                BitStream announce = BitStream();
                announce.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                std::string system_message = "\033[33m[SYSTEM]\033[37m ";
                system_message+= name.C_String();
                system_message+= "\033[32m connected to the server.\033[37m\n";
                announce.Write(RakString(system_message.c_str()));
                peer->Send(&announce, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
            }
        }
            break;
		case ID_NEW_INCOMING_CONNECTION:
        {
            printf("A connection is incoming.\n");
        }
            break;
		case ID_DISCONNECTION_NOTIFICATION:
        {
            printf("A client has disconnected.\n");
            if (isGuidRegistered(packet->guid))
            {
                BitStream announce = BitStream();
                announce.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                std::string system_message = "\033[33m[SYSTEM]\033[37m ";
                {
                    std::lock_guard<std::mutex> lock(listMutex);
                    for (const auto& [guid, user] : connectionList_)
                    {
                        if (user.userGUID == packet->guid)
                        {
                            system_message += user.Name.c_str();
                        }
                    }
                }
                system_message += "\033[31m disconnected from the server.\033[37m\n";
                announce.Write(system_message.c_str());
                peer->Send(&announce, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                connectionList_.erase(packet->guid);
            }
        }
			break;
		case ID_CONNECTION_LOST:
        {
			printf("A client lost connection.\n");
            if (!isGuidRegistered(packet->guid)) break;
            BitStream announce = BitStream();
            announce.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
            std::string system_message = "\033[33m[SYSTEM]\033[37m ";
            {
                std::lock_guard<std::mutex> lock(listMutex);
                for (const auto& [guid, user] : connectionList_)
                {
                    if(user.userGUID == packet->guid)
                    {
                        system_message+=user.Name.c_str();
                    }
                }
            }
            system_message+= "\033[31m lost connection to the server.\033[37m\n";
            announce.Write(system_message.c_str());
            peer->Send(&announce, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
            connectionList_.erase(packet->guid);
        }
			break;
        case ID_CHAT_MESSAGE:
        {
            if (!isGuidRegistered(packet->guid)) break;
            BitStream bsIn = BitStream(packet->data, packet->length, false);
            bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
            RakString message;
            bsIn.Read(message);
            BitStream bsOut = BitStream();
            bsOut.Write((RakNet::MessageID)ID_CHAT_MESSAGE);
            {
                std::lock_guard<std::mutex> lock(listMutex);
                for (const auto& [guid, user] : connectionList_)
                {
                    if(user.userGUID == packet->guid)
                    {
                        bsOut.Write(user.Name.c_str());
                    }
                }
            }
            
            bsOut.Write(message);

            peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
        }
            break;

        case ID_NO_FREE_INCOMING_CONNECTIONS:
		    printf("The server is full.\n");
			break;

        case ID_VOICE_DATA:
        {
            uint16_t len = 0;
            BitStream bsIn = BitStream(packet->data, packet->length, false);
            bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
            bsIn.Read(len);
            if(len)
            {
                uint8_t __data[512];
                bsIn.Read(reinterpret_cast<char*>(__data), static_cast<unsigned int>(len));

                BitStream bsOut = BitStream();
                bsOut.Write((RakNet::MessageID)ID_VOICE_DATA);
                bsOut.Write(packet->guid.g);
                bsOut.Write(len);
                bsOut.Write(reinterpret_cast<const char*>(__data), (uint16_t)len);
                peer->Send(&bsOut, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 1, packet->systemAddress, true);
            }   
        }
    }
}

void RakChatServer::MainThread()
{   
    while(isServerRunning)
    {
        for (packet=peer->Receive(); packet; peer->DeallocatePacket(packet), packet=peer->Receive())
		{
            
            HandlePacket(packet);
		}
    }
}



int main()
{
    RakChatServer* theServer = new RakChatServer();
    while(true)
    {

    }   
    return 0;
}