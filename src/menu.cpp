#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <unistd.h>
#include "cartridge.hpp"
#include "menu.hpp"

namespace GUI {

using namespace std;


Entry::Entry(string label, function<void()> callback, function<bool()> enabled_check)
    : callback(callback), enabled_check(enabled_check)
{
    set_label(label);
}

Entry::~Entry()
{
    SDL_DestroyTexture(whiteTexture);
    SDL_DestroyTexture(redTexture);
    SDL_DestroyTexture(greyTexture);
}

void Entry::set_label(string label)
{
    this->label = label;

    if (whiteTexture != nullptr) SDL_DestroyTexture(whiteTexture);
    if (redTexture   != nullptr) SDL_DestroyTexture(redTexture);
    if (greyTexture  != nullptr) SDL_DestroyTexture(greyTexture);

    whiteTexture = gen_text(label, { 255, 255, 255 });
    redTexture   = gen_text(label, { 255,   0,   0 });
    greyTexture  = gen_text(label, { 100, 100, 100 });
}

void Entry::render(int x, int y) {
    if (!is_enabled()) {
        render_texture(greyTexture, x, y);
    } else {
        render_texture(selected ? redTexture : whiteTexture, x, y);
    }
}

ControlEntry::ControlEntry(string action, SDL_Scancode* key) : key(key),
    Entry::Entry(
        action,
        [&]{ keyEntry->set_label(SDL_GetScancodeName(*(this->key) = query_key())); })
{
    this->keyEntry = new Entry(SDL_GetScancodeName(*key), []{});
}

static string btn_name(int b) {
    const char* s = SDL_GameControllerGetStringForButton((SDL_GameControllerButton)b);
    return s ? s : "?";
}

ControlEntry::ControlEntry(string action, int* button) : button(button),
    Entry::Entry(
        action,
        [&]{ keyEntry->set_label(btn_name(*(this->button) = query_button())); })
{
    this->keyEntry = new Entry(btn_name(*button), []{});
}

CycleEntry::CycleEntry(string prefix, int* value, int min_val, int max_val,
                       function<void(int)> on_change, string suffix,
                       function<bool()> enabled_check)
    : Entry("", [this]{ cycle(); }, enabled_check),
      value(value), min_val(min_val), max_val(max_val),
      prefix(prefix), suffix(suffix), on_change(on_change)
{
    update_label();
}

void CycleEntry::cycle()
{
    (*value)++;
    if (*value > max_val) *value = min_val;
    on_change(*value);
    update_label();
}

void CycleEntry::update_label()
{
    set_label(prefix + to_string(*value) + suffix);
}

ToggleEntry::ToggleEntry(string prefix, bool* value, function<void(bool)> on_change,
                         function<bool()> enabled_check)
    : Entry("", [this]{ toggle(); }, enabled_check),
      value(value), prefix(prefix), on_change(on_change)
{
    update_label();
}

void ToggleEntry::toggle()
{
    *value = !(*value);
    on_change(*value);
    update_label();
}

void ToggleEntry::update_label()
{
    set_label(prefix + (*value ? "On" : "Off"));
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
    clear_error();
    cursor = 0;
    top = 0;
    bottom = MAX_ENTRY;
}

void Menu::clear_error()
{
    delete errorMessage;
    errorMessage = nullptr;
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

void Menu::jump_to(int newCursor)
{
    if (newCursor < 0 || newCursor >= entries.size())
        return;

    entries[cursor]->unselect();
    cursor = newCursor;

    // Update visible window to show the new cursor
    if (cursor < top) {
        top = cursor;
        bottom = top + MAX_ENTRY;
    } else if (cursor >= bottom) {
        bottom = cursor + 1;
        top = bottom - MAX_ENTRY;
    }

    entries[cursor]->select();
    clear_error();
}

void Menu::update(u8 const* keys)
{
    int oldCursor = cursor;

    if (keys[SDL_SCANCODE_PAGEDOWN])
    {
        int target = cursor;
        for (int i = 0; i < MAX_ENTRY && target < entries.size() - 1; i++) {
            target++;
            while (target < entries.size() - 1 && !entries[target]->is_enabled())
                target++;
        }
        if (entries[target]->is_enabled())
            jump_to(target);
    }
    else if (keys[SDL_SCANCODE_PAGEUP])
    {
        int target = cursor;
        for (int i = 0; i < MAX_ENTRY && target > 0; i++) {
            target--;
            while (target > 0 && !entries[target]->is_enabled())
                target--;
        }
        if (entries[target]->is_enabled())
            jump_to(target);
    }
    else if (keys[SDL_SCANCODE_DOWN] and cursor < entries.size() - 1)
    {
        do {
            cursor++;
            if (cursor == bottom) {
                bottom += 1;
                top += 1;
            }
        } while (cursor < entries.size() - 1 && !entries[cursor]->is_enabled());

        // If we ended on a disabled entry, revert
        if (!entries[cursor]->is_enabled()) {
            cursor = oldCursor;
        }
        clear_error();
    }
    else if (keys[SDL_SCANCODE_UP] and cursor > 0)
    {
        do {
            cursor--;
            if (cursor < top) {
                top -= 1;
                bottom -= 1;
            }
        } while (cursor > 0 && !entries[cursor]->is_enabled());

        // If we ended on a disabled entry, revert
        if (!entries[cursor]->is_enabled()) {
            cursor = oldCursor;
        }
        clear_error();
    }

    if (oldCursor != cursor) {
        entries[oldCursor]->unselect();
        entries[cursor]->select();
    }

    if (keys[SDL_SCANCODE_RETURN] && entries[cursor]->is_enabled())
        entries[cursor]->trigger();
}

void Menu::render()
{
    for (int i = top; i < entries.size() && i < bottom; ++i)
    {
        int y = (i - top) * FONT_SZ;
        entries[i]->render(TEXT_CENTER, y);
    }

    if (errorMessage != nullptr)
        errorMessage->render(TEXT_CENTER, HEIGHT - FONT_SZ * 3 / 2);
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
                          [=]{ load_rom(path); }));
    }
    closedir(dp);
    sort_by_label();
}

void FileMenu::load_rom(string path)
{
    clear_error();
    Cartridge::load(path.c_str());
    if (!Cartridge::loaded())
    {
        errorMessage = new Entry("Load failed");
        return errorMessage->select();
    }
    toggle_pause();
}

FileMenu::FileMenu()
{
    char cwd[512];
    change_dir(getcwd(cwd, 512));
}

void FileMenu::update(u8 const* keys)
{
    // Call base update for standard navigation
    Menu::update(keys);

    // Letter jump: find first entry starting with pressed letter
    for (int scancode = SDL_SCANCODE_A; scancode <= SDL_SCANCODE_Z; scancode++)
    {
        if (keys[scancode])
        {
            char letter = 'a' + (scancode - SDL_SCANCODE_A);

            // Search from current position forward, then wrap around
            for (int offset = 1; offset < entries.size(); offset++)
            {
                int idx = (cursor + offset) % entries.size();
                string label = entries[idx]->get_label();

                if (!label.empty() && tolower(label[0]) == letter && entries[idx]->is_enabled())
                {
                    jump_to(idx);
                    return;
                }
            }
        }
    }
}


}
