#include "RakNetTypes.h"
#include <unordered_map>
#include <vector> 
#include <string>

struct RakChatUser
{
    std::string Name;
    RakNet::SystemAddress userAddr;
    RakNet::RakNetGUID userGUID;
    bool operator==(const RakChatUser& user)
    {
        if (this->Name != user.Name) return false;
        if (this->userAddr != user.userAddr) return false;
        if (this->userGUID != user.userGUID) return false;
        return true;
    }
};


class RakChatUserPool
{
public:
    RakChatUserPool();
    ~RakChatUserPool();
    uint16_t insert(const RakChatUser& user);
    bool remove(uint16_t userid);
    bool exists(uint16_t userid);
    uint16_t getId(const RakNet::RakNetGUID& guid);
    const std::string& getName(const RakNet::RakNetGUID& guid);
    RakChatUser* get(uint16_t userid);
    RakChatUser* get(const RakNet::RakNetGUID& guid);
    RakChatUser* get(const RakNet::SystemAddress& systemAddress);
    RakChatUser* get(const std::string& userName);
private:
    std::unordered_map<uint16_t, RakChatUser> connectionList_;
    std::vector<uint16_t> freeIds;
    uint16_t nextId = 1;
};
