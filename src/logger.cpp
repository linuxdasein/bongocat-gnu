#include "header.hpp"

extern "C" {
#include <SDL2/SDL.h>
}

#define BONGO_ERROR 1

namespace logger {

std::unique_ptr<ILogger> g_logger;

class CombinedLogger : public ILogger
{
public:
    //
    void attach(std::unique_ptr<ILogger> logger) {
        sinks.emplace_back(std::move(logger));
    }
    //
    void log(std::string message, Severity level) override {
        for(auto& sink : sinks)
            sink->log(message, level);
    }

private:
    std::vector<std::unique_ptr<ILogger>> sinks;
};

class StreamLogger : public ILogger
{
public:
    StreamLogger(std::ostream& s)
        : ost(s) {}

    void log(std::string message, Severity level) override {
        std::string tag;

        switch(level) {
            case Severity::critical:
                tag = "[Fatal error]: ";
                break;
            case Severity::medium:
                tag = "[Error]: ";
                break;
            case Severity::info:
                tag = "[Info]: ";
                break;
            case Severity::debug:
                tag = "[Debug]: ";
                break;
        }

        ost << tag << message << std::endl;
    }
private:
    std::ostream& ost;
};

class MsgboxLogger : public ILogger
{
public:
    void log(std::string message, Severity level) override {
        if(level != Severity::critical)
            return;

        SDL_MessageBoxButtonData buttons[] = {
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Retry" },
            { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Cancel" },
        };

        SDL_MessageBoxColorScheme colorScheme = {{
            /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 255, 255,255 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            { 0, 0, 0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 0, 0, 0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            { 255,255, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 128, 128, 128 }
        }};

        std::string title = "Fatal error";

        SDL_MessageBoxData messagebox_data = {
    	    SDL_MESSAGEBOX_ERROR,
    	    NULL,
    	    title.c_str(),
    	    message.c_str(),
    	    SDL_arraysize(buttons),
    	    buttons,
    	    &colorScheme
        };

        int button_id;

        SDL_ShowMessageBox(&messagebox_data, &button_id);

        if (button_id == -1 || button_id == 1) {
            exit(BONGO_ERROR);
        }
    }
};

void init() {
    auto tmp = std::make_unique<CombinedLogger>();
    auto e_logger = std::make_unique<StreamLogger>(std::cerr);
    auto m_logger = std::make_unique<MsgboxLogger>();
    tmp->attach(std::move(e_logger));
    tmp->attach(std::move(m_logger));
    g_logger = std::move(tmp);
}

ILogger& get() {
    return *g_logger;
}

}