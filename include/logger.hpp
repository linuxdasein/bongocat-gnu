#include "header.hpp"

namespace logger
{

// Interface for
class GlobalLogger : public ILogger
{
public:

    void attach(std::unique_ptr<ILogger> logger);

    void log(std::string message, Severity level) override;

    static void init(int width, int height);

    static GlobalLogger& get();

private:
    std::vector<std::unique_ptr<ILogger>> sinks;
};

class SfmlOverlayLogger : public ILogger, public sf::Drawable
{
public:
    SfmlOverlayLogger(int w, int h);

    void log(std::string message, Severity level) override;

    void draw(sf::RenderTarget& target, sf::RenderStates rst) const override;

    void set_visible(bool value);

private:
    bool is_visible = false;
    sf::RectangleShape background;
    std::list<sf::Text> log_text;
};

}