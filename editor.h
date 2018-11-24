#ifndef EDITOR_H
#define EDITOR_H

WINDOW *editor_start_menu();

WINDOW *map_editor();

WINDOW *characters_dialog();

WINDOW *add_character_dialog();

WINDOW *regions_dialog();

WINDOW *add_region_dialog();

WINDOW *tile_to_region();

WINDOW *region_to_character();

WINDOW *rename_region_dialog();

WINDOW *edit_character_dialog();

WINDOW *validate_dialog();

WINDOW *edit_time_dialog();

#endif				/* EDITOR_H */
