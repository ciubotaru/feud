#ifndef GAMEFILE_H
#define GAMEFILE_H

#define SAVE_DIRNAME "/.feud"
#define SAVE_FILENAME "/feud.sav"
#define LOG_FILENAME "/feud.log"

void clearlog();

int add_to_chronicle(char *format, ...);

unsigned int load_game();

unsigned int save_game();

char **load_namelist(const char *filename, const char *placeholder, const int size);

int savefile_exists();

void delete_savefile();

void rename_logfile();

#endif				/* GAMEFILE_H */
