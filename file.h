#ifndef GAMEFILE_H
#define GAMEFILE_H

#define SAVE_DIRNAME "/.feud"
#define SAVE_FILENAME "/feud.sav"
#define LOG_FILENAME "/feud.log"

int add_to_chronicle(char *format, ...);

unsigned int load_game();

unsigned int save_game();

#endif				/* GAMEFILE_H */
