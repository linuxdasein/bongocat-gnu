#include "header.hpp"

namespace logger
{

// logger class implementing a collection of loggers
class GlobalLogger : public ILogger
{
public:

    void attach(std::unique_ptr<ILogger> logger);

    void log(std::string message, Severity level) override;

    static void init();

    static GlobalLogger& get();

private:
    std::vector<std::unique_ptr<ILogger>> sinks;
};

// sfml overlay logger class
class SfmlOverlayLogger : public ILogger, public sf::Drawable
{
public:
    SfmlOverlayLogger(int w, int h);

    void log(std::string message, Severity level) override;

    void draw(sf::RenderTarget& target, sf::RenderStates rst) const override;

    void set_visible(bool value);

    void set_size(sf::Vector2u size);

private:
    bool is_visible = false;
    sf::RectangleShape background;
    std::list<sf::Text> log_text;
};

}