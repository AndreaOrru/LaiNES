#pragma once
#include <functional>
#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace GUI {


struct MenuEntry
{
    SDL_Color white = { 255, 255, 255 };
    SDL_Color red   = { 255,   0,   0 };

    std::string label;
    std::function<void()> callback;

    SDL_Texture* selected;
    SDL_Texture* unselected;

    MenuEntry(std::string label, std::function<void()> callback);
    ~MenuEntry();
};

class Menu
{
  protected:
    std::vector<MenuEntry*> entries;
    int cursor = 0;

  public:
    void add(std::string label, std::function<void()> callback = []{});
    void update(u8 const* keys);
    virtual void render();
    virtual Menu* reset();
};

class FileMenu : public Menu
{
    void change_dir(std::string dir);

  public:
    void render();
    Menu* reset();
    FileMenu();
};


}
