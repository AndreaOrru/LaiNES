#pragma once
#include <functional>
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "gui.hpp"

namespace GUI {


class Entry
{
    std::string label;
    std::function<void()> callback;

    bool selected = false;
    SDL_Texture* whiteTexture = nullptr;
    SDL_Texture* redTexture   = nullptr;

  public:
    Entry(std::string label, std::function<void()> callback = []{});
    ~Entry();

    void set_label(std::string label);
    inline std::string& get_label() { return label; }

    virtual void select()   { selected = true;  };
    virtual void unselect() { selected = false; };
    void trigger() { callback(); };
    virtual void render(int x, int y);
};

class ControlEntry : public Entry
{
    SDL_Scancode* key;
    int* button;
    Entry* keyEntry;

  public:
    ControlEntry(std::string action, SDL_Scancode* key);
    ControlEntry(std::string action, int* button);
    void select()    { Entry::select();   keyEntry->select();   }
    void unselect()  { Entry::unselect(); keyEntry->unselect(); }
    void render(int x, int y)    { Entry::render(x, y);   keyEntry->render(TEXT_RIGHT, y);   }
};

class Menu
{
    const int MAX_ENTRY = GUI::HEIGHT / FONT_SZ - 1;
    int cursor = 0;
    int top = 0;
    int bottom = MAX_ENTRY;

  public:
    std::vector<Entry*> entries;

    void add(Entry* entry);
    void clear();
    void sort_by_label();
    void update(u8 const* keys);
    void render();
};

class FileMenu : public Menu
{
    void change_dir(std::string dir);

  public:
    FileMenu();
};


}
