#include "logger.hpp"
#include "header.hpp"

namespace logger {

std::unique_ptr<GlobalLogger> g_logger;

void GlobalLogger::attach(std::unique_ptr<ILogger> logger) {
    sinks.emplace_back(std::move(logger));
}
    
void GlobalLogger::log(std::string message, Severity level) {
    for(auto& sink : sinks)
        sink->log(message, level);
}

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

SfmlOverlayLogger::SfmlOverlayLogger(int w, int h)
    : background(sf::Vector2f(w, h)) {
    background.setFillColor(sf::Color(0, 0, 0, 128));
}

void SfmlOverlayLogger::log(std::string message, Severity level) {
    sf::Text log_message(message, data::get_debug_font(), 14);
    const float line_height = log_message.getLocalBounds().height;
    log_message.setPosition(10.0f, 4.0f + log_text.size() * line_height);

    switch(level) {
    case Severity::medium:
        log_message.setFillColor(sf::Color::Yellow);
        break;
    case Severity::critical:
        log_message.setFillColor(sf::Color::Red);
        break;
    default:
        log_message.setFillColor(sf::Color::White);
        break;
    }

    if(log_message.getGlobalBounds().top + line_height > background.getSize().y)
        log_text.clear();

    log_text.push_back(log_message);

    if(Severity::critical == level) {
        is_visible = true;
    }
}

void SfmlOverlayLogger::draw(sf::RenderTarget& target, sf::RenderStates rst) const {
    if(!is_visible)
        return;

    target.draw(background, rst);
    for(auto &m : log_text)
        target.draw(m, rst);
}

void SfmlOverlayLogger::set_visible(bool value) {
    is_visible = value;
}

void GlobalLogger::init(int w, int h) {
    auto tmp = std::make_unique<GlobalLogger>();
    auto e_logger = std::make_unique<StreamLogger>(std::cerr);
    auto o_logger = std::make_unique<SfmlOverlayLogger>(w, h);
    tmp->attach(std::move(e_logger));
    tmp->attach(std::move(o_logger));
    g_logger = std::move(tmp);
}

ILogger& get() {
    return *g_logger;
}

GlobalLogger& GlobalLogger::get() {
    return *g_logger;
}

}