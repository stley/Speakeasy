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

    RakChatChannel rootCh = RakChatChannel("Root", nullptr, 0);

    RakChatChannel testCh = RakChatChannel("Test", nullptr, 0);
    uint16_t rootId = channelPool.CreateChannel(rootCh);
    uint16_t testId = channelPool.CreateChannel(testCh);
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
                RakChatUser theUser(peer);
                theUser.userAddr = packet->systemAddress;
                theUser.userGUID = packet->guid;
                theUser.Name = name.C_String();
                uint16_t newId = userPool.insert(theUser);
                
                response.Write((RakNet::MessageID)ID_REGISTER_ME);
                response.Write((unsigned char)'Y');
                response.Write(newId);
                peer->Send(&response, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);

                char buf[9];
                std::sprintf(buf, "(%d) ", newId);
                std::string system_message = buf;
                system_message+= name.C_String();
                system_message+= "\033[32m connected to the server.\033[37m\n";

                userPool.BroadcastSystemMessage(system_message.c_str(), theUser.userGUID);

                channelPool.GetChannel(0)->JoinChannel(userPool.get(newId));
            }
            break;
        }
            
		case ID_NEW_INCOMING_CONNECTION:
        {
            printf("A connection is incoming.\n");
            break;
        }
            
		case ID_DISCONNECTION_NOTIFICATION:
        {
            printf("A client has disconnected.\n");
            RakChatUser* user = userPool.get(packet->guid);
            if (user != nullptr)
            {
                BitStream announce = BitStream();
                announce.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                char buf[9];
                std::sprintf(buf, "(%d) ", userPool.getId(packet->guid));
                std::string system_message = buf;
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
                char buf[9];
                std::sprintf(buf, "(%d) ", userPool.getId(packet->guid));
                std::string system_message = buf;
                system_message+= user->Name.c_str();
                system_message+= "\033[31m lost connection to the server.\033[37m\n";
                announce.Write(system_message.c_str());
                peer->Send(&announce, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
                userPool.remove( userPool.getId(packet->guid) );
            }
            break;
        }
			
        case ID_CHAT_MESSAGE:
        {
            if (!isGuidRegistered(packet->guid)) break;
		    RakChatChannel* chan = channelPool.IsUserInAnyChannel(userPool.get(packet->guid));
            if (!chan) break;

            BitStream bsIn = BitStream(packet->data, packet->length, false);
            bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
            RakString message;
            bsIn.Read(message);

            BitStream bsOut = BitStream();
            bsOut.Write((RakNet::MessageID)ID_CHAT_MESSAGE);
            RakString author(userPool.getName(packet->guid).c_str());
            bsOut.Write(author);
            bsOut.Write(message);
            
            chan->Broadcast(&bsOut);

            //peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
            break;
        }
            

        case ID_NO_FREE_INCOMING_CONNECTIONS:
		    printf("The server is full.\n");
			break;

        case ID_VOICE_DATA:
        {
            if (!isGuidRegistered(packet->guid)) break;
            RakChatUser* theUser = userPool.get(packet->guid);
            RakChatChannel* chan = channelPool.IsUserInAnyChannel(theUser);
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
                chan->Broadcast(&bsOut, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 1, theUser);
                //peer->Send(&bsOut, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 1, packet->systemAddress, true);
            }
            break;
        }

        case ID_QUERY:
        {
            RakChatUser* theUser = userPool.get(packet->guid);
            if (!theUser)
                break;

            BitStream bsIn = BitStream(packet->data, packet->length, false);
            bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
            unsigned char result;
            if (bsIn.Read(result) && result == 'P')
            {
                BitStream peerBS = BitStream();
                peerBS.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                RakString rs = RakString("List of connected peers:");
                peerBS.Write(rs);
                theUser->SendBitStream(&peerBS);
                
                for (const auto& [ uid, chatUser ] : userPool.GetPeerList())
                {
                    peerBS.Reset();
                    peerBS.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                    rs = RakString("(%d) - %s", uid, chatUser.Name.c_str());
                    peerBS.Write(rs);
                    theUser->SendBitStream(&peerBS);
                }
            }
            else if (result == 'C')
            {
                BitStream peerBS = BitStream();
                peerBS.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                RakString rs = RakString("List of available channels:");
                peerBS.Write(rs);
                theUser->SendBitStream(&peerBS);

                for (const auto& [chanid, chan] : channelPool.GetList())
                {
                    peerBS.Reset();
                    peerBS.Write((RakNet::MessageID)ID_SYSTEM_MESSAGE);
                    rs = RakString("(%d) - %s", chanid, chan.Name().c_str());
                    peerBS.Write(rs);
                    theUser->SendBitStream(&peerBS);
                }
            }


            break;
        }

        case ID_CHANNEL_ACTION:
        {
            RakChatUser* theUser = userPool.get(packet->guid);
            if (!theUser)
                break;

            BitStream bsIn = BitStream(packet->data, packet->length, false);
            bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
            unsigned char result;
            if (bsIn.Read(result) && result == 'J')
            {
                uint16_t chanId = 0;
                bsIn.Read(chanId);

                RakChatChannel* oldChan = channelPool.IsUserInAnyChannel(theUser);
                RakChatChannel* newChan = channelPool.GetChannel(chanId);

                if (newChan)
                {   if (oldChan)
                        oldChan->LeaveChannel(theUser, LEAVE_GRACEFULLY);
                    newChan->JoinChannel(theUser);
                }
            }
            break;
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