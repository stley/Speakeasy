#include "GUIClient.hpp"
#include <algorithm>

static int text_height(mu_Font font) {
  return r_get_text_height();
}
static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(text, len);
}

static float bg[3] = { 90, 95, 100 };

std::array<char, 256> button_map = {};

std::array<char, 256> key_map = {};

static void init_maps()
{
    button_map[SDL_BUTTON_LEFT & 0xff] = MU_MOUSE_LEFT;
    button_map[SDL_BUTTON_RIGHT & 0xff] = MU_MOUSE_RIGHT;
    button_map[SDL_BUTTON_MIDDLE & 0xff] = MU_MOUSE_MIDDLE;

    key_map[SDLK_LSHIFT       & 0xff] = MU_KEY_SHIFT;
    key_map[SDLK_RSHIFT       & 0xff] = MU_KEY_SHIFT;
    key_map[SDLK_LCTRL        & 0xff] = MU_KEY_CTRL;
    key_map[SDLK_RCTRL        & 0xff] = MU_KEY_CTRL;
    key_map[SDLK_LALT         & 0xff] = MU_KEY_ALT;
    key_map[SDLK_RALT         & 0xff] = MU_KEY_ALT;
    key_map[SDLK_RETURN       & 0xff] = MU_KEY_RETURN;
    key_map[SDLK_BACKSPACE    & 0xff] = MU_KEY_BACKSPACE;
}



void GUIClient::Resize(int width, int height)
{
   r_resize(width, height);
}


void GUIClient::Initialize()
{
    init_maps();
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "SDL INIT ERROR: " << SDL_GetError() << "\n";
        return;
    }
    r_init();
    //assert(window_ != nullptr);

    //window_ = SDL_CreateWindow( "Speakeasy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    

    

    //context_ = SDL_GL_CreateContext(window_);
    //if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    /*{
        std::cout << "Failed to init GLAD\n";
        return;
    }*/
    /*if (!window_)
    {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return;
    }*/
    
    ctx = static_cast<mu_Context*>(malloc(sizeof(mu_Context)));

    mu_init(ctx);

    ctx->text_width = text_width;
    ctx->text_height = text_height;


    running_ = true;
    while (running_)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT: exit(EXIT_SUCCESS); break;
                case SDL_MOUSEMOTION: mu_input_mousemove(ctx, event.motion.x, event.motion.y); break;
                case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, event.wheel.y * -30); break;
                case SDL_TEXTINPUT: mu_input_text(ctx, event.text.text); break;
                            
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    int b = button_map[event.button.button & 0xff];
                    if (b && event.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, event.button.x, event.button.y, b); }
                    if (b && event.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, event.button.x, event.button.y, b);   }
                    break;
                }
                
                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                int c = key_map[event.key.keysym.sym & 0xff];
                if (c && event.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
                if (c && event.type ==   SDL_KEYUP) { mu_input_keyup(ctx, c);   }
                    break;
                }
            }    
        }


        ProcessGUI();

        


        /* render */
        r_clear(mu_color(bg[0], bg[1], bg[2], 255));
        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
          switch (cmd->type) {
            case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
            case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
            case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
            case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
          }
        }
        r_present();
    }
    SDL_Quit();
}

void GUIClient::ProcessGUI()
{
    mu_begin(ctx);
    switch (under_->GetClientState())
    {
        
        case FailedConnection:
        {
            if (under_) delete under_;
            under_ = new RakChatClient();
            PromptInitWindow(); break;
        }
        case RegistrationFailed:
        {
            if (under_) delete under_;
            under_ = new RakChatClient();
            PromptInitWindow(); break;
        }
        case Init:
        case Connecting:
        case Registering:
            PromptInitWindow(); break;

        case Connected: PromptMainWindow(); break;
    }
    mu_end(ctx);
}
void GUIClient::PromptInitWindow()
{
    static std::string winTitle;

    switch (under_->GetClientState())
    {
        case Connecting: break;
        case Init: winTitle = "Speakeasy Client"; break;
        case FailedConnection: winTitle = "Connection failed!"; break;
        case Registering: winTitle = "Connection established. Registering..."; break;
        case RegistrationFailed: winTitle = "Registration Failed."; break;
    }

    Resize(300, 300);
    if (mu_begin_window_ex(ctx, winTitle.c_str(), mu_rect(0, 0, 300, 300), MU_OPT_NOCLOSE | MU_OPT_NORESIZE | MU_OPT_NOINTERACT | MU_OPT_NOSCROLL))
    {
        //int submit = 0;
        static char ipAddr[20];
        static char svPort[7];
        static char myUsername[64];
        int label_widths[] = {130, 100, 100};
        int widths[] = {100, 100, 100};

        mu_layout_row(ctx, 3, label_widths, 0);
        mu_label(ctx, "");
        mu_label(ctx, "IP:");
        mu_layout_row(ctx, 2, widths, 0);
        mu_label(ctx, "");
        if(mu_textbox(ctx, ipAddr, sizeof(ipAddr))) {}

        mu_layout_row(ctx, 2, label_widths, 0);
        mu_label(ctx, "");
        mu_label(ctx, "Port:");
        mu_layout_row(ctx, 2, widths, 0);
        mu_label(ctx, "");
        if(mu_textbox(ctx, svPort, sizeof(svPort))) {}
        mu_layout_row(ctx, 3, label_widths, 0);
        mu_label(ctx, "");
        mu_label(ctx, "Username:");
        mu_layout_row(ctx, 2, widths, 0);
        mu_label(ctx, "");
        if(mu_textbox(ctx, myUsername, sizeof(myUsername))) {}

        mu_layout_row(ctx, 2, widths, 0);
        mu_label(ctx, "");
        mu_label(ctx, "");
        mu_layout_row(ctx, 2, widths, 0);
        mu_label(ctx, "");
        if(mu_button(ctx, "Connect"))
        {
            unsigned short port = std::clamp(std::atoi(svPort), 0, 65535);
            if (port != 0)
            {
                under_->ClientConfigure(ipAddr, port, myUsername);
                under_->ClientConnect();
            }
            winTitle.clear();
            winTitle += "Connecting to ";
            winTitle += ipAddr;
            winTitle += ":";
            winTitle += svPort;
            winTitle += "...";
        }
    }
    mu_end_window(ctx);
}

void GUIClient::PromptMainWindow()
{
    Resize(800, 600);

    if (mu_begin_window_ex(ctx, "", mu_rect(0, 0, 800, 600), MU_OPT_NORESIZE | MU_OPT_NOINTERACT | MU_OPT_NOSCROLL))
    {
        
    }
    mu_end_window(ctx);
}

int main()
{
    GUIClient visual_;

    visual_.Initialize();
    return 0;
}

void GUIClient::ConsolePrint(const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Message out;
    out.msg = buffer;
    {
        std::lock_guard<std::mutex> lock(messagesMutex_);
        messages_.emplace_back(out);
        if(messages_.size() > 500)
            messages_.pop_front();
    }
}