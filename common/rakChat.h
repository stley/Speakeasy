//#pragma once

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakString.h"
#include "RakNetTypes.h"  // MessageID



#if !defined RKC_MAX_CLIENTS
    #define RKC_MAX_CLIENTS (64);
#endif


enum GameMessages
{
    ID_SYSTEM_MESSAGE=ID_USER_PACKET_ENUM+1,
    ID_REGISTER_ME,
    ID_CHAT_MESSAGE,
    ID_VOICE_DATA
};



