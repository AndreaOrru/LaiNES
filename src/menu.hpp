#pragma once
#include <functional>
#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace GUI {


class Entry
{
    int x, y;

    std::string label;
    std::function<void()> callback;

    bool selected = false;
    SDL_Texture* whiteTexture;
    SDL_Texture* redTexture;

  public:
    Entry(std::string label, std::function<void()> callback = []{}, int x = -1, int y = -1);
    ~Entry();

    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }
    int getX() { return x; }
    int getY() { return y; }

    void select()   { selected = true;  };
    void unselect() { selected = false; };
    void trigger()  { callback(); };
    virtual void render();
};

class Menu
{
    std::vector<Entry*> entries;
    int cursor = 0;

  public:
    void add(Entry* entry);
    void clear();
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
