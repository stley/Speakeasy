#include "Client.hpp"



int main()
{
    
    while (clientInit)
    {
        clientInit = false;
        RakChatClient theClient;
        theClient.ClientMain();
    }
    return 0;
}