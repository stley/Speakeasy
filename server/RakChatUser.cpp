#include "RakChatUser.hpp"

RakChatUserPool::RakChatUserPool()
{
}

RakChatUserPool::~RakChatUserPool()
{
}

uint16_t RakChatUserPool::insert(const RakChatUser &user)
{
    uint16_t id = 0;

    if (!freeIds.empty())
    {
        id = freeIds.front();
        freeIds.erase(freeIds.begin());
    }
    else
    {
        id = nextId++;
    }

    connectionList_.emplace(id, user);

    return id;
}

bool RakChatUserPool::remove(uint16_t userid)
{
    auto it = connectionList_.find(userid);

    if (it != connectionList_.end())
    {
        connectionList_.erase(userid);
        freeIds.push_back(userid);
    }
    return false;
}
bool RakChatUserPool::exists(uint16_t userid)
{   
    auto it = connectionList_.find(userid);

    if (it != connectionList_.end())
        return true;   

    return false;
}
uint16_t RakChatUserPool::getId(const RakNet::RakNetGUID &guid)
{
    for (const auto& [id, user] : connectionList_)
    {
        if(user.userGUID == guid)
        {
            return id;
        }
    }
    return 0;
}
const std::string& RakChatUserPool::getName(const RakNet::RakNetGUID &guid)
{
    std::string ret = "null";
    for (const auto& [id, user] : connectionList_)
    {
        if(user.userGUID == guid)
        {
            return user.Name;
        }
    }
}
RakChatUser *RakChatUserPool::get(uint16_t userid)
{
    auto it = connectionList_.find(userid);
    if (it != connectionList_.end())
    {
        return &it->second;
    }
    return nullptr;
}

RakChatUser *RakChatUserPool::get(const RakNet::RakNetGUID& guid)
{
    for (auto& [id, user] : connectionList_)
    {
        if (user.userGUID == guid)
            return &user;
    }
    
    return nullptr;
}

RakChatUser *RakChatUserPool::get(const RakNet::SystemAddress& systemAddress)
{
    for (auto& [id, user] : connectionList_)
    {
        if(user.userAddr == systemAddress)
            return &user;
    }

    return nullptr;
}

RakChatUser *RakChatUserPool::get(const std::string& userName)
{
    for (auto& [id, user] : connectionList_)
    {
        if(user.Name == userName)
            return &user;
    }

    return nullptr;
}