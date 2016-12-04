#include <cstdlib>
#include <SimpleIni.h>
#include "config.hpp"
#include "gui.hpp"

namespace GUI {

/* Settings */
CSimpleIniA ini(true, false, false);

/* Window settings */
int last_window_size = 1;

/* Controls settings */
SDL_Scancode KEY_A     [] = { SDL_SCANCODE_A,      SDL_SCANCODE_ESCAPE };
SDL_Scancode KEY_B     [] = { SDL_SCANCODE_S,      SDL_SCANCODE_ESCAPE };
SDL_Scancode KEY_SELECT[] = { SDL_SCANCODE_SPACE,  SDL_SCANCODE_ESCAPE };
SDL_Scancode KEY_START [] = { SDL_SCANCODE_RETURN, SDL_SCANCODE_ESCAPE };
SDL_Scancode KEY_UP    [] = { SDL_SCANCODE_UP,     SDL_SCANCODE_ESCAPE };
SDL_Scancode KEY_DOWN  [] = { SDL_SCANCODE_DOWN,   SDL_SCANCODE_ESCAPE };
SDL_Scancode KEY_LEFT  [] = { SDL_SCANCODE_LEFT,   SDL_SCANCODE_ESCAPE };
SDL_Scancode KEY_RIGHT [] = { SDL_SCANCODE_RIGHT,  SDL_SCANCODE_ESCAPE };
int BTN_UP    [] = { -1, -1 };
int BTN_DOWN  [] = { -1, -1 };
int BTN_LEFT  [] = { -1, -1 };
int BTN_RIGHT [] = { -1, -1 };
int BTN_A     [] = { -1, -1 };
int BTN_B     [] = { -1, -1 };
int BTN_SELECT[] = { -1, -1 };
int BTN_START [] = { -1, -1 };
bool useJoystick[] = { false, false };



/* Ensure config directory exists */
const char* get_config_path(char * buf, int buflen)
{
    /* Bail on the complex stuff if we don't need it */
    if (!USE_CONFIG_DIR)
    {
       return CONFIG_FALLBACK;
    }

    /* First, get the home directory */
    char homepath[CONFIG_PATH_MAX];
    char path[CONFIG_PATH_MAX];
    char * home = getenv("HOME");
    if (home == NULL)
        return CONFIG_FALLBACK;

    snprintf(homepath, sizeof(homepath), "%s/.config", home);

    /* Then, .config as a folder */
    int res = mkdir(homepath, CONFIG_DIR_DEFAULT_MODE);
    int err = errno;

    if (res == -1 && err != EEXIST)
        return CONFIG_FALLBACK;

    snprintf(path, sizeof(path), "%s/%s", homepath, CONFIG_DIR_NAME);

    /* Finally, CONFIG_DIR_NAME as a sub-folder */
    res = mkdir(path, CONFIG_DIR_DEFAULT_MODE);
    err = errno;

    if (res == -1 && err != EEXIST)
        return CONFIG_FALLBACK;

    snprintf(buf, buflen, "%s/settings", path);

    return buf;
}


/* Load settings */
void load_settings()
{
    /* Files */
    char path[CONFIG_PATH_MAX];
    ini.LoadFile(get_config_path(path, sizeof(path)));

    /* Screen settings */
    int screen_size = atoi(ini.GetValue("screen", "size", "1"));
    if (screen_size < 1 || screen_size > 4)
        screen_size = 1;

    set_size(screen_size);

    /* Control settings */
    for (int p = 0; p < 1; p++)
    {
        const char* section = p==0?"controls p1":"controls p2";

        useJoystick[p] = (ini.GetValue(section, "usejoy", "no"))[0] == 'y';
        if (useJoystick[p])
        {
            BTN_UP[p] = atoi(ini.GetValue(section, "UP", "-1"));
            BTN_DOWN[p] = atoi(ini.GetValue(section, "DOWN", "-1"));
            BTN_LEFT[p] = atoi(ini.GetValue(section, "LEFT", "-1"));
            BTN_RIGHT[p] = atoi(ini.GetValue(section, "RIGHT", "-1"));
            BTN_A[p] = atoi(ini.GetValue(section, "A", "-1"));
            BTN_B[p] = atoi(ini.GetValue(section, "B", "-1"));
            BTN_SELECT[p] = atoi(ini.GetValue(section, "SELECT", "-1"));
            BTN_START[p] = atoi(ini.GetValue(section, "START", "-1"));
        } else {
            KEY_UP[p] = (SDL_Scancode)atoi(ini.GetValue(section, "UP", "82"));
            KEY_DOWN[p] = (SDL_Scancode)atoi(ini.GetValue(section, "DOWN", "81"));
            KEY_LEFT[p] = (SDL_Scancode)atoi(ini.GetValue(section, "LEFT", "80"));
            KEY_RIGHT[p] = (SDL_Scancode)atoi(ini.GetValue(section, "RIGHT", "79"));
            KEY_A[p] = (SDL_Scancode)atoi(ini.GetValue(section, "A", "4"));
            KEY_B[p] = (SDL_Scancode)atoi(ini.GetValue(section, "B", "22"));
            KEY_SELECT[p] = (SDL_Scancode)atoi(ini.GetValue(section, "SELECT", "44"));
            KEY_START[p] = (SDL_Scancode)atoi(ini.GetValue(section, "START", "40"));
        }
    }
}


/* Save settings */
void save_settings()
{
    /* Screen settings */
    char buf[2] = {0};
    sprintf(buf, "%d", last_window_size);
    ini.SetValue("screen", "size", buf);

    /* Control settings */
    for (int p = 0; p < 1; p++)
    {
        const char* section = p==0?"controls p1":"controls p2";

        sprintf(buf, "%d", useJoystick[p]?BTN_UP[p]:KEY_UP[p]);
        ini.SetValue("section", "UP", buf);
        sprintf(buf, "%d", useJoystick[p]?BTN_DOWN[p]:KEY_DOWN[p]);
        ini.SetValue("section", "DOWN", buf);
        sprintf(buf, "%d", useJoystick[p]?BTN_LEFT[p]:KEY_LEFT[p]);
        ini.SetValue("section", "LEFT", buf);
        sprintf(buf, "%d", useJoystick[p]?BTN_RIGHT[p]:KEY_RIGHT[p]);
        ini.SetValue("section", "RIGHT", buf);
        sprintf(buf, "%d", useJoystick[p]?BTN_A[p]:KEY_A[p]);
        ini.SetValue("section", "A", buf);
        sprintf(buf, "%d", useJoystick[p]?BTN_B[p]:KEY_B[p]);
        ini.SetValue("section", "B", buf);
        sprintf(buf, "%d", useJoystick[p]?BTN_SELECT[p]:KEY_SELECT[p]);
        ini.SetValue("section", "SELECT", buf);
        sprintf(buf, "%d", useJoystick[p]?BTN_START[p]:KEY_START[p]);
        ini.SetValue("section", "START", buf);
        ini.SetValue("section", "usejoy", useJoystick[p]?"yes":"no");
    }
   
    char path[CONFIG_PATH_MAX];
    ini.SaveFile(get_config_path(path, sizeof(path)));
}

}
