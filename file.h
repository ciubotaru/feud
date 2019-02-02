#ifndef GAMEFILE_H
#define GAMEFILE_H

#define SAVE_DIRNAME "/.feud"
#define SAVE_FILENAME "/feud.sav"
#define LOG_FILENAME "/feud.log"

int add_to_chronicle(char *format, ...);

unsigned int load_game();

unsigned int save_game();

char **load_region_names(const int size);

char **load_character_names(const int size);

#endif				/* GAMEFILE_H */
