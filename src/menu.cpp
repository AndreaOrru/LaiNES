#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include "cartridge.hpp"
#include "menu.hpp"

namespace GUI {

using namespace std;


Entry::Entry(string label, function<void()> callback) : callback(callback)
{
    set_label(label);
}

Entry::~Entry()
{
    SDL_DestroyTexture(whiteTexture);
    SDL_DestroyTexture(redTexture);
}

void Entry::set_label(string label)
{
    this->label = label;

    if (whiteTexture != nullptr) SDL_DestroyTexture(whiteTexture);
    if (redTexture   != nullptr) SDL_DestroyTexture(redTexture);

    whiteTexture = gen_text(label, { 255, 255, 255 });
    redTexture   = gen_text(label, { 255,   0,   0 });
}

void Entry::render(int x, int y) {
    render_texture(selected ? redTexture : whiteTexture, x, y);
}

ControlEntry::ControlEntry(string action, SDL_Scancode* key) : key(key),
    Entry::Entry(
        action,
        [&]{ keyEntry->set_label(SDL_GetScancodeName(*(this->key) = query_key())); })
{
    this->keyEntry = new Entry(SDL_GetScancodeName(*key), []{});
}

ControlEntry::ControlEntry(string action, int* button) : button(button),
    Entry::Entry(
        action,
        [&]{ keyEntry->set_label(to_string(*(this->button) = query_button())); })
{
    this->keyEntry = new Entry(to_string(*button), []{});
}


void Menu::add(Entry* entry)
{
    if (entries.empty())
        entry->select();
    entries.push_back(entry);
}

void Menu::clear()
{
    for (auto entry : entries)
        delete entry;
    entries.clear();
    cursor = 0;
}

void Menu::sort_by_label()
{
    if (entries.empty())
        return;
    entries[0]->unselect();
    sort(entries.begin(), entries.end(), [](Entry* a, Entry* b) {
        return a->get_label() < b->get_label();
    });
    entries[0]->select();
}

void Menu::update(u8 const* keys)
{
    int oldCursor = cursor;

    if (keys[SDL_SCANCODE_DOWN] and cursor < entries.size() - 1)
    {
        cursor++;
        if (cursor == bottom) {
            bottom += 1;
            top += 1;
        }
    }
    else if (keys[SDL_SCANCODE_UP] and cursor > 0)
    {
        cursor--;
        if (cursor < top) {
            top -= 1;
            bottom -= 1;
        }
    }
    entries[oldCursor]->unselect();
    entries[cursor]->select();

    if (keys[SDL_SCANCODE_RETURN])
        entries[cursor]->trigger();
}

void Menu::render()
{
    for (int i = top; i < entries.size() && i < bottom; ++i)
    {
        int y = (i - top) * FONT_SZ;
        entries[i]->render(TEXT_CENTER, y);
    }
}

void FileMenu::change_dir(string dir)
{
    clear();

    struct dirent* dirp;
    DIR* dp = opendir(dir.c_str());

    while ((dirp = readdir(dp)) != NULL)
    {
        string name = dirp->d_name;
        string path = dir + "/" + name;

        if (name[0] == '.' and name != "..") continue;

        if (dirp->d_type == DT_DIR)
            add(new Entry(name + "/",
                          [=]{ change_dir(path); }));

        else if (name.size() > 4 and name.substr(name.size() - 4) == ".nes")
            add(new Entry(name,
                          [=]{ Cartridge::load(path.c_str()); toggle_pause(); }));
    }
    closedir(dp);
    sort_by_label();
}

FileMenu::FileMenu()
{
    char cwd[512];

    change_dir(getcwd(cwd, 512));
}


}
