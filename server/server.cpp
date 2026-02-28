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


bool RakChatServer::isNameAvailable(const char* name, size_t len)
{   
    std::string getName(name, len);
    if (userPool.get(getName) != nullptr) return false;
    return true;
}

bool RakChatServer::isGuidRegistered(RakNetGUID guid_)
{
    RakChatUser* usr = userPool.get(guid_);
    if (usr != nullptr) return true;
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
            if(!isNameAvailable(name.C_String(), name.GetLength()))
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
                uint16_t newId = userPool.insert(theUser);
                
                response.Write((RakNet::MessageID)ID_REGISTER_ME);
                response.Write((unsigned char)'Y');
                response.Write(newId);
                peer->Send(&response, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
                BitStream announce = BitStream();
                announce.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                std::string system_message = "\033[33m[SYSTEM]\033[37m ";
                char buf[9];
                std::sprintf(buf, "(%d) ", newId);
                system_message+= buf;
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
            RakChatUser* user = userPool.get(packet->guid);
            if (user != nullptr)
            {
                BitStream announce = BitStream();
                announce.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                std::string system_message = "\033[33m[SYSTEM]\033[37m ";
                system_message += user->Name.c_str();
                system_message += "\033[31m disconnected from the server.\033[37m\n";
                announce.Write(system_message.c_str());
                peer->Send(&announce, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                userPool.remove( userPool.getId(packet->guid) );
            }
        }
			break;
		case ID_CONNECTION_LOST:
        {
			printf("A client lost connection.\n");
            if (!isGuidRegistered(packet->guid)) break;
            
            RakChatUser* user = userPool.get(packet->guid);
            if (user != nullptr)
            {
                BitStream announce = BitStream();
                announce.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                std::string system_message = "\033[33m[SYSTEM]\033[37m ";
                system_message+= user->Name.c_str();
                system_message+= "\033[31m lost connection to the server.\033[37m\n";
                announce.Write(system_message.c_str());
                peer->Send(&announce, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                userPool.remove( userPool.getId(packet->guid) );
            }  
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
            RakString author(userPool.getName(packet->guid).c_str());
            bsOut.Write(author);
            bsOut.Write(message);

            peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
        }
            break;

        case ID_NO_FREE_INCOMING_CONNECTIONS:
		    printf("The server is full.\n");
			break;

        case ID_VOICE_DATA:
        {
            if (!isGuidRegistered(packet->guid)) break;
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
                uint16_t uID = userPool.getId(packet->guid);
                bsOut.Write(uID);
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