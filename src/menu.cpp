#include <dirent.h>
#include <unistd.h>
#include "cartridge.hpp"
#include "gui.hpp"
#include "menu.hpp"

namespace GUI {

using namespace std;


MenuEntry::MenuEntry(string label, function<void()> callback, int x)
{
    this->label = label;
    this->callback = callback;
    this->x = x;
    unselected = gen_text(label, white);
    selected   = gen_text(label, red);
}

MenuEntry::~MenuEntry()
{
    SDL_DestroyTexture(unselected);
    SDL_DestroyTexture(selected);
}

void MenuEntry::regen()
{
    SDL_DestroyTexture(unselected);
    SDL_DestroyTexture(selected);
    unselected = gen_text(label, white);
    selected   = gen_text(label, red);
}


void Menu::add(string label, function<void()> callback, int x)
{
    entries.push_back(new MenuEntry(label, callback, x));
}

void Menu::update(u8 const* keys)
{
    if (keys[SDL_SCANCODE_DOWN] and cursor < entries.size() - 1)
        cursor++;
    else if (keys[SDL_SCANCODE_UP] and cursor > 0)
        cursor--;
    else if (keys[SDL_SCANCODE_RETURN])
        entries[cursor]->callback();
}

void Menu::render()
{
    int h; SDL_QueryTexture(entries[0]->selected, NULL, NULL, NULL, &h);

    for (int i = 0; i < entries.size(); i++)
        render_texture(entries[i]->unselected, entries[i]->x, i * h);
    render_texture(entries[cursor]->selected, entries[cursor]->x, cursor * h);
}

void Menu::regen()
{
    for (auto entry : entries)
        entry->regen();
}

Menu* Menu::reset()
{
    cursor = 0;
    return this;
}


void FileMenu::change_dir(string dir)
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
            add(name + "/", [=]{ change_dir(path); }, 10);
        else if (name.size() > 4 and name.substr(name.size() - 4) == ".nes")
            add(name, [=]{ Cartridge::load(path.c_str()); toggle_pause(); }, 10);
    }
    closedir(dp);
}

Menu* FileMenu::reset()
{
    change_dir(getwd(NULL));
    return this;
}

FileMenu::FileMenu()
{
    reset();
}


}
