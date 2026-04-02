#include "Client.hpp"
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT  
#define NK_INCLUDE_FONT_BAKING            
#define SDL_MAIN_HANDLED
//#include <glad/glad.h>
#include <SDL.h>
extern "C"
{
    #include "render.h"
    #include "microui.h"
}
#include <SDL_opengl.h>
#include <queue>
#include <stdarg.h>
#include <mutex>


struct Message
{
    std::string msg;
};


class GUIClient 
{
private:
    //SDL_Window* window_ = nullptr;
    //SDL_GLContext context_;
    std::atomic<bool> running_ = false;
    SDL_Event event;
    RakChatClient* under_ = nullptr;
    mu_Context *ctx = nullptr;

    std::deque<Message> messages_;
    std::mutex messagesMutex_;

    std::vector<std::string> buffer_;


    void ProcessGUI();
public:
    void Resize(int width, int height);
    GUIClient()
    {
        under_ = new RakChatClient();
    }
    ~GUIClient()
    {
        if (under_)
            delete under_;
    }

    void Initialize();

    void PromptInitWindow();
    void PromptMainWindow();

    void ConsolePrint(const char* fmt, ...);
};