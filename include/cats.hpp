// Interface for cats

#pragma once

#include <data.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <memory>
#include <SFML/Graphics.hpp>
#include <json/json.h>

#include <list>
#include <set>

namespace cats
{

enum class CatModeId
{
    osu,
    taiko,
    ctb,
    mania,
    custom
};

class ICat : public sf::Drawable
{
public:

    // Initilizes the cat
    virtual bool init(const data::Settings& st, const Json::Value& cfg) = 0;

    // Updates cat's state, called per frame
    virtual void update() {}

    // Virtual destructor
    virtual ~ICat() {}
};

class MousePaw 
{
protected:

    // Initialize mouse paw with json config
    bool init(const Json::Value& mouse_cfg, const Json::Value& paw_cfg);

    // Update device and paw position according to the mouse_pos
    void update_paw_position(std::pair<double, double> mouse_pos);

    // Display paw represented by its coordinates pss2
    void draw_paw(sf::RenderTarget& target, sf::RenderStates rst) const;

    // Set offset and scale for mouse sprite
    void set_mouse_parameters(sf::Vector2i offset, double scale);

    sf::Sprite device, left_button, right_button;
private:
    // draw an arc about an array of points
    void draw_arc(sf::RenderTarget& target, sf::RenderStates rst, sf::Color color, float width) const;

    double scale = 1.0;
    sf::Vector2i offset = {0, 0};

    sf::Vector2i paw_start;
    sf::Vector2i paw_end;
    sf::Vector2i A, B, C;

    sf::Color paw_color;
    sf::Color paw_edge_color;
    std::vector<sf::Vector2f> pss2;
};

class CatKeyboardGroup : public sf::Drawable {
public:
    void init(const Json::Value& keys_config);

    void update();
    
    void draw(sf::RenderTarget& target, sf::RenderStates rst) const override;

private:
    class Key {
    public:
        Key(int i, bool p)
            : id(i), persistent(p) {}

        void add_codes(std::set<int> cds) {
            codes.merge(cds);
        }

        int get_id() const {
            return id; 
        }

        bool is_pressed() const;

        bool is_persistent() const {
            return persistent;
        }

        bool is_combined() const {
            return codes.size() > 1;
        }

    private:
        int id;
        bool persistent;
        std::set<int> codes;
    };

    std::list<Key> released_keys;
    std::list<Key> combined_keys;
    std::list<Key> pressed_keys;
    std::list<Key> persistent_keys;
    std::unique_ptr<sf::Drawable> def_kbg;
    std::vector<std::unique_ptr<sf::Drawable>> sprites;
    std::map<int, sf::Drawable*> key_actions;
};

class CatKeyboardGroup;

class CustomCat : public ICat, private MousePaw
{
public:

    bool init(const data::Settings& st, const Json::Value& cfg) override;
    void update() override;
    void draw(sf::RenderTarget& target, sf::RenderStates rst) const override;

private:
    bool init_mouse(const Json::Value& mouse_config);

private:

    sf::Sprite bg;
    std::list<std::unique_ptr<CatKeyboardGroup>> kbd_groups;

    bool is_mouse, is_mouse_on_top;
};

std::unique_ptr<ICat> get_cat(cats::CatModeId mode);

}
