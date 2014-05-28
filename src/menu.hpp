#pragma once
#include <dirent.h>
#include <functional>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <vector>
#include "cartridge.hpp"
#include "gui.hpp"

namespace GUI {


using namespace std;

struct MenuEntry
{
    SDL_Color white = { 255, 255, 255 };
    SDL_Color red   = { 255,   0,   0 };

    string label;
    function<void()> callback;

    SDL_Texture* selected;
    SDL_Texture* unselected;

    MenuEntry(string label, function<void()> callback = []{})
    {
        this->label = label;
        this->callback = callback;
        unselected = gen_text(label, white);
        selected   = gen_text(label, red);
    }

    ~MenuEntry()
    {
        SDL_DestroyTexture(selected);
        SDL_DestroyTexture(unselected);
    }
};

class Menu
{
  protected:
    vector<MenuEntry*> entries;
    int cursor = 0;

  public:
    void add(string label, function<void()> callback = []{})
    {
        entries.push_back(new MenuEntry(label, callback));
    }

    void update(u8 const* keys)
    {
        if (keys[SDL_SCANCODE_DOWN] and cursor < entries.size() - 1)
            cursor++;
        else if (keys[SDL_SCANCODE_UP] and cursor > 0)
            cursor--;
        else if (keys[SDL_SCANCODE_RETURN])
            entries[cursor]->callback();
    }

    void render()
    {
        for (int i = 0; i < entries.size(); i++)
            render_texture(entries[i]->unselected, -1, i * fontPt);
        render_texture(entries[cursor]->selected, -1, cursor * fontPt);
    }

    virtual Menu* reset()
    {
        cursor = 0;
        return this;
    }
};

class FileMenu : public Menu
{
    void change_dir(string dir)
    {
        for (auto entry : entries)
            delete entry;
        entries.clear();
        cursor = 0;

        struct dirent* dirp;
        DIR* dp = opendir(dir.c_str());

        while ((dirp = readdir(dp)) != NULL)
        {
            string name = dirp->d_name;
            string path = dir + "/" + name;

            if (name[0] == '.' and name != "..") continue;
            if (dirp->d_type == DT_DIR)
                add(name + "/", [=]{ change_dir(path); });
            else
                add(name, [=]{ Cartridge::load(path.c_str()); toggle_pause(); });
        }
        closedir(dp);
    }

  public:
    Menu* reset()
    {
        change_dir(getwd(NULL));
        return this;
    }

    FileMenu()
    {
        reset();
    }
};


}
