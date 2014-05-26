#pragma once
#include <vector>
#include "gui.hpp"

namespace GUI {


struct MenuEntry
{
    SDL_Color white = { 255, 255, 255 };
    SDL_Color red   = { 255,   0,   0 };

    std::string label;
    SDL_Texture* selected;
    SDL_Texture* unselected;

    MenuEntry(std::string label)
    {
        this->label = label;
        unselected = gen_text(label, white);
        selected   = gen_text(label, red);
    }
};

class Menu
{
    std::vector<MenuEntry*> entries;
    int cursor = 0;

  public:
    Menu(std::vector<std::string> labels)
    {
        for (auto label : labels)
            entries.push_back(new MenuEntry(label));
    }

    void update(u8 const* keys)
    {
        if (keys[SDL_SCANCODE_DOWN] and cursor < entries.size() - 1)
            cursor++;
        else if (keys[SDL_SCANCODE_UP] and cursor > 0)
            cursor--;
    }

    void render()
    {
        for (int i = 0; i < entries.size(); i++)
            render_texture(entries[i]->unselected, -1, i * fontPt);
        render_texture(entries[cursor]->selected, -1, cursor * fontPt);
    }
};


}
