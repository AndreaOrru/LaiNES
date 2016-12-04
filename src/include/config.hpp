#pragma once
#include <cerrno>
#include <sys/stat.h>
#include <SDL2/SDL.h>

#define CONFIG_DIR_DEFAULT_MODE      S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH
#define USE_CONFIG_DIR true
#define CONFIG_DIR_NAME "LaiNES" 
#define CONFIG_FALLBACK ".laines-settings"
/* PATH_MAX is a portability nightmare. */
#define CONFIG_PATH_MAX 1024 

namespace GUI {

/* Loading and saving */
void load_settings();
void save_settings();
const char* get_config_path(char * buf, int buflen);

extern int last_window_size;
extern SDL_Scancode KEY_A     [];
extern SDL_Scancode KEY_B     [];
extern SDL_Scancode KEY_SELECT[];
extern SDL_Scancode KEY_START [];
extern SDL_Scancode KEY_UP    [];
extern SDL_Scancode KEY_DOWN  [];
extern SDL_Scancode KEY_LEFT  [];
extern SDL_Scancode KEY_RIGHT [];
extern int BTN_UP    [];
extern int BTN_DOWN  [];
extern int BTN_LEFT  [];
extern int BTN_RIGHT [];
extern int BTN_A     [];
extern int BTN_B     [];
extern int BTN_SELECT[];
extern int BTN_START [];
extern bool useJoystick[];

}
