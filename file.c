#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <math.h>	/* for log10() */
#include <unistd.h>	/* for R_OK */
#include <ctype.h>	/* for isalnum */
#include <time.h>

#if defined(__linux__) || defined(__CYGWIN__)

#include <endian.h>

#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define htobe16(x) OSSwapHostToBigInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)

#else

#error Platform not supported

#endif

#include <stdarg.h>
#include "world.h"

#define GAME_METADATA_SIZE (2 * sizeof(unsigned char) + 4 * sizeof(uint16_t))	/* year, mon, next character id, next piece id, selected_character, moves left */
#define CHARACTERLIST_METADATA_SIZE (sizeof(uint16_t))	/* nr of characters */
#define CHARACTERLIST_UNIT_SIZE (sizeof(uint16_t) + 17 + sizeof(uint16_t) + sizeof(unsigned char) + sizeof(uint16_t) + sizeof(unsigned char) + sizeof(uint16_t) + sizeof(unsigned char))	/* id, name, money, rank, birth_year, birth_mon, death_year, death_mon */
#define REGIONLIST_METADATA_SIZE (sizeof(uint16_t))	/* nr of regions */
#define REGIONLIST_UNIT_SIZE (sizeof(uint16_t) + 17 + sizeof(uint16_t))	/* id, name, owner */
#define GRID_METADATA_SIZE (sizeof(uint16_t) * 2)	/* grid height and width */
#define GRID_UNIT_SIZE (sizeof(unsigned char) + sizeof(uint16_t))	/* walkable, next_region_id */
#define PIECES_METADATA_SIZE (sizeof(uint16_t))	/* nr of pieces */
#define PIECES_UNIT_SIZE (sizeof(uint16_t) + sizeof(unsigned char) + sizeof(uint16_t) * 3)	/* id, type, height, width, owner_nr */
#define FEUDAL_METADATA_SIZE (sizeof(uint16_t))	/* nr of lord-vassal relations */
#define FEUDAL_UNIT_SIZE (sizeof(uint16_t) * 2)	/* vassal, lord */
#define HEIR_METADATA_SIZE (sizeof(uint16_t))	/* nr of characters with a heir */
#define HEIR_UNIT_SIZE (sizeof(uint16_t) * 2)	/* grantor, heir */
#define DIPLOMACY_METADATA_SIZE (sizeof(uint16_t))	/* nr of dipstatuses */
#define DIPLOMACY_UNIT_SIZE (sizeof(uint16_t) * 2 + sizeof(char))	/* character1_id, character2_id, status */
#define DIPOFFER_METADATA_SIZE (sizeof(uint16_t))	/* nr of dipoffers */
#define DIPOFFER_UNIT_SIZE (sizeof(uint16_t) * 2 + sizeof(char))	/* from, to, offer */

static uint16_t selected_character_id;

#define strconcat(...) (char *) strconcat_( count_arguments(#__VA_ARGS__), __VA_ARGS__)

inline static unsigned int count_arguments(char *s)
{
	unsigned i, argc = 1;
	for (i = 0; s[i]; i++)
		if (s[i] == ',')
			argc++;
	return argc;
}

static char *strconcat_(unsigned int count, ...)
{
	unsigned int i;
	size_t len = 0;
	va_list args;
	va_start(args, count);
	for (i = 0; i < count; i++) {
		const char *tmp = va_arg(args, char *);
		len += strlen(tmp);
	}
	va_end(args);

	char *output = malloc(len + 1);
	if (output == NULL) exit(EXIT_FAILURE);

	char *dst = output;
	va_start(args, count);
	for (i = 0; i < count; i++) {
		const char *src = va_arg(args, char *);
		while (*dst++ = *src++);
		dst--;
	}
	va_end(args);
	return output;
}

void clearlog() {
	if (!getenv("HOME")) return;
	struct stat st = { 0 };
	char *logdir = strconcat(getenv("HOME"), SAVE_DIRNAME);
	if (stat(logdir, &st) == -1) {
		mkdir(logdir, 0700);
	}
	char *logfile = strconcat(logdir, LOG_FILENAME);
	free(logdir);

	FILE *fp = fopen(logfile, "w");
	free(logfile);
	if (fp == NULL) return;
	fclose(fp);
}

int add_to_chronicle(char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	vsprintf(world->message, format, argptr);
	va_end(argptr);

	if (!getenv("HOME"))
		return 1;
	char *logfile = strconcat(getenv("HOME"), SAVE_DIRNAME, LOG_FILENAME);

	FILE *fp = fopen(logfile, "a");
	free(logfile);
	if (fp == NULL)
		return 3;

	char date_char[100];
	sprintf(date_char, "%s of year %d: ",
		months[world->current_time.tm_mon],
		world->current_time.tm_year);

	fwrite(date_char, sizeof(char), strlen(date_char), fp);

	int bytes_written = fwrite(world->message, sizeof(char), strlen(world->message), fp);
	fclose(fp);
	if (bytes_written == 0)
		return 1;
	return 0;
}

int deserialize_game_metadata(char **buffer, int *pos)
{
	uint16_t year_be = 0;
	unsigned char mon = 0;
	unsigned char moves_left = 0;
	memcpy(&year_be, *buffer, sizeof(uint16_t));
	memcpy(&mon, *buffer + sizeof(uint16_t), 1);
	set_gametime(be16toh(year_be), mon);
	uint16_t next_character_id_be = 0;
	memcpy(&next_character_id_be, *buffer + sizeof(uint16_t) + 1,
	       sizeof(uint16_t));
	world->next_character_id = be16toh(next_character_id_be);
	uint16_t next_piece_id_be = 0;
	memcpy(&next_piece_id_be, *buffer + 1 + 2 * sizeof(uint16_t),
	       sizeof(uint16_t));
	world->next_piece_id = be16toh(next_piece_id_be);
	uint16_t selected_character_be = 0;
	memcpy(&selected_character_be, *buffer + 1 + 3 * sizeof(uint16_t),
	       sizeof(uint16_t));
	selected_character_id = be16toh(selected_character_be);
	memcpy(&moves_left, *buffer + 1 + 4 * sizeof(uint16_t),
	       sizeof(unsigned char));
	world->moves_left = moves_left;
	*pos = GAME_METADATA_SIZE + 1;
	return 0;
}

int deserialize_characterlist(char **buffer, int *pos)
{
	character_t *current = NULL;
	uint16_t nr_characters_be = 0;
	int buffer_pos = *pos;
	memcpy(&nr_characters_be, *buffer + buffer_pos, CHARACTERLIST_METADATA_SIZE);
	uint16_t nr_characters = be16toh(nr_characters_be);
/**
	if (nr_characters == 0) {
		buffer_pos = *pos + CHARACTERLIST_METADATA_SIZE;
		*pos = buffer_pos + 1;
		return;
	}
**/
	int i;
	buffer_pos = *pos + CHARACTERLIST_METADATA_SIZE;
	uint16_t id_be, money_be, birthdate_year_be, deathdate_year_be;
	char name[17];
	int offset = 0;

	for (i = 0; i < nr_characters; i++) {
		offset = 0;
		memcpy(&id_be, *buffer + buffer_pos + offset, sizeof(uint16_t));	/* id */
		offset += sizeof(uint16_t);
		strcpy(name, *buffer + buffer_pos + offset);
		offset += 17;
		current = add_character(name);
		current->id = be16toh(id_be);
		world->next_character_id--;	//return back after adding a character
		memcpy(&money_be, *buffer + buffer_pos + offset,
		       sizeof(uint16_t));
		offset += sizeof(uint16_t);
		set_money(current, be16toh(money_be));
		memcpy(&(current->rank), *buffer + buffer_pos + offset,
		       sizeof(unsigned char));
		offset += sizeof(unsigned char);
		memcpy(&birthdate_year_be, *buffer + buffer_pos + offset,
		       sizeof(uint16_t));
		offset += sizeof(uint16_t);
		current->birthdate.tm_year = be16toh(birthdate_year_be);
		memcpy(&(current->birthdate.tm_mon),
		       *buffer + buffer_pos + offset, 1);
		offset += 1;
		memcpy(&deathdate_year_be, *buffer + buffer_pos + offset,
		       sizeof(uint16_t));
		offset += sizeof(uint16_t);
		current->deathdate.tm_year = be16toh(deathdate_year_be);
		memcpy(&(current->deathdate.tm_mon),
		       *buffer + buffer_pos + offset, 1);
		buffer_pos += CHARACTERLIST_UNIT_SIZE;
	}
	*pos = buffer_pos + 1;
	return 0;
}

int deserialize_regionlist(char **buffer, int *pos)
{
	world->next_region_id = 1;
	region_t *current = NULL;
	uint16_t nr_regions_be = 0;
	int buffer_pos = *pos;
	memcpy(&nr_regions_be, *buffer + buffer_pos, REGIONLIST_METADATA_SIZE);
	uint16_t nr_regions = be16toh(nr_regions_be);	/* global var */

	int i;
	buffer_pos = *pos + REGIONLIST_METADATA_SIZE;
	uint16_t id_be, owner_be;
	char name[17];
	for (i = 0; i < nr_regions; i++) {
		memcpy(&id_be, *buffer + buffer_pos, sizeof(uint16_t));	/* id */
//              memcpy(&size_be, *buffer + buffer_pos + sizeof(uint16_t), sizeof(uint16_t));
		strcpy(name, *buffer + buffer_pos + sizeof(uint16_t));
		memcpy(&owner_be, *buffer + buffer_pos + sizeof(uint16_t) + 17,
		       sizeof(uint16_t));
		current = add_region(name);
		current->id = be16toh(id_be);
		current->size = 0;	//be16toh(size_be);
//              current->owner = get_character_by_id(be16toh(owner_be));
		change_region_owner(get_character_by_id(be16toh(owner_be)),
				    current);
		buffer_pos += REGIONLIST_UNIT_SIZE;
	}
	*pos = buffer_pos + 1;
	return 0;
}

int deserialize_grid(char **buffer, int *pos)
{
	uint16_t height_be = 0;
	uint16_t width_be = 0;
	int buffer_pos = *pos;
	memcpy(&height_be, *buffer + buffer_pos, sizeof(uint16_t));
	memcpy(&width_be, *buffer + buffer_pos + sizeof(uint16_t),
	       sizeof(uint16_t));
	uint16_t height = be16toh(height_be);
	uint16_t width = be16toh(width_be);
	if (world->grid != NULL)
		remove_grid();
	world->grid = create_grid(height, width);
	if (!world->grid)
		return 1;

	int i, j;
	buffer_pos = *pos + GRID_METADATA_SIZE;
	uint16_t region_be;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			memcpy(&world->grid->tiles[i][j]->walkable,
			       *buffer + buffer_pos, 1);
			memcpy(&region_be, *buffer + buffer_pos + 1, sizeof(uint16_t));	/* id */
			change_tile_region(get_region_by_id(be16toh(region_be)),
					   world->grid->tiles[i][j]);
			buffer_pos += GRID_UNIT_SIZE;
		}
	}
	*pos = buffer_pos + 1;
	return 0;
}

int deserialize_pieces(char **buffer, int *pos)
{
	piece_t *current = NULL;
	uint16_t nr_pieces_be;
	int buffer_pos = *pos;
	memcpy(&nr_pieces_be, *buffer + buffer_pos, sizeof(uint16_t));
	uint16_t nr_pieces = be16toh(nr_pieces_be);
	int i;
	buffer_pos = *pos + PIECES_METADATA_SIZE;
	unsigned char type;
	uint16_t id_be, height_be, width_be, owner_be;
	uint16_t height, width, owner;
	for (i = 0; i < nr_pieces; i++) {
		memcpy(&id_be, *buffer + buffer_pos, sizeof(uint16_t));
		memcpy(&type, *buffer + buffer_pos + sizeof(uint16_t), 1);
		memcpy(&height_be, *buffer + buffer_pos + 1 + sizeof(uint16_t),
		       sizeof(uint16_t));
		memcpy(&width_be,
		       *buffer + buffer_pos + 1 + 2 * sizeof(uint16_t),
		       sizeof(uint16_t));
		memcpy(&owner_be,
		       *buffer + buffer_pos + 1 + 3 * sizeof(uint16_t),
		       sizeof(uint16_t));
		height = be16toh(height_be);
		width = be16toh(width_be);
		owner = be16toh(owner_be);
		current =
		    add_piece(type, height, width,
			      get_character_by_id(owner));
		if (!current)
			return 1;
		current->id = be16toh(id_be);
		world->next_piece_id = current->id;
		world->grid->tiles[height][width]->
		    piece = current;
		buffer_pos += PIECES_UNIT_SIZE;
	}
	*pos = buffer_pos + 1;
	return 0;
}

int deserialize_feudal(char **buffer, int *pos)
{
	character_t *current = NULL;
	uint16_t nr_feudal_be = 0;
	int buffer_pos = *pos;
	memcpy(&nr_feudal_be, *buffer + buffer_pos, FEUDAL_METADATA_SIZE);
	uint16_t nr_feudal = be16toh(nr_feudal_be);

	int i;
	buffer_pos = *pos + FEUDAL_METADATA_SIZE;
	uint16_t vassal_id_be, lord_id_be;

	for (i = 0; i < nr_feudal; i++) {
		memcpy(&vassal_id_be, *buffer + buffer_pos, sizeof(uint16_t));	/* vassal_id */
		current = get_character_by_id(be16toh(vassal_id_be));
		memcpy(&lord_id_be, *buffer + buffer_pos + sizeof(uint16_t), sizeof(uint16_t));	/* lord_id */
		current->lord = get_character_by_id(be16toh(lord_id_be));
		buffer_pos += FEUDAL_UNIT_SIZE;
	}
	*pos = buffer_pos + 1;
	return 0;
}

int deserialize_heir(char **buffer, int *pos)
{
	character_t *current = NULL;
	uint16_t nr_heir_be = 0;
	int buffer_pos = *pos;
	memcpy(&nr_heir_be, *buffer + buffer_pos, HEIR_METADATA_SIZE);
	uint16_t nr_heir = be16toh(nr_heir_be);

	int i;
	buffer_pos = *pos + HEIR_METADATA_SIZE;
	uint16_t character_id_be, heir_id_be;

	for (i = 0; i < nr_heir; i++) {
		memcpy(&character_id_be, *buffer + buffer_pos, sizeof(uint16_t));	/* character_id */
		current = get_character_by_id(be16toh(character_id_be));
		memcpy(&heir_id_be, *buffer + buffer_pos + sizeof(uint16_t), sizeof(uint16_t));	/* heir_id */
		current->heir = get_character_by_id(be16toh(heir_id_be));
		buffer_pos += HEIR_UNIT_SIZE;
	}
	*pos = buffer_pos + 1;
	return 0;
}

int deserialize_diplomacy(char **buffer, int *pos)
{
	uint16_t nr_dipstat_be = 0;
	int buffer_pos = *pos;
	memcpy(&nr_dipstat_be, *buffer + buffer_pos, DIPLOMACY_METADATA_SIZE);
	uint16_t nr_dipstat = be16toh(nr_dipstat_be);
	unsigned char status;

	int i;
	buffer_pos = *pos + DIPLOMACY_METADATA_SIZE;
	uint16_t character1_id_be, character2_id_be;
	character_t *character1 = NULL, *character2 = NULL;

	for (i = 0; i < nr_dipstat; i++) {
		memcpy(&character1_id_be, *buffer + buffer_pos, sizeof(uint16_t));	/* character1_id */
		character1 = get_character_by_id(be16toh(character1_id_be));
		memcpy(&character2_id_be, *buffer + buffer_pos + sizeof(uint16_t), sizeof(uint16_t));	/* character2_id */
		character2 = get_character_by_id(be16toh(character2_id_be));
		memcpy(&status, *buffer + buffer_pos + sizeof(uint16_t) * 2,
		       sizeof(unsigned char));
		set_diplomacy(character1, character2, (unsigned int)status);
		buffer_pos += DIPLOMACY_UNIT_SIZE;
	}
	*pos = buffer_pos + 1;
	return 0;
}

int deserialize_dipoffer(char **buffer, int *pos)
{
	uint16_t nr_dipoffer_be = 0;
	int buffer_pos = *pos;
	memcpy(&nr_dipoffer_be, *buffer + buffer_pos, DIPOFFER_METADATA_SIZE);
	uint16_t nr_dipoffer = be16toh(nr_dipoffer_be);
	unsigned char offer;

	int i;
	buffer_pos = *pos + DIPOFFER_METADATA_SIZE;
	uint16_t from_id_be, to_id_be;
	character_t *from = NULL, *to = NULL;

	for (i = 0; i < nr_dipoffer; i++) {
		memcpy(&from_id_be, *buffer + buffer_pos, sizeof(uint16_t));	/* from_id */
		from = get_character_by_id(be16toh(from_id_be));
		memcpy(&to_id_be, *buffer + buffer_pos + sizeof(uint16_t), sizeof(uint16_t));	/* to_id */
		to = get_character_by_id(be16toh(to_id_be));
		memcpy(&offer, *buffer + buffer_pos + sizeof(uint16_t) * 2,
		       sizeof(unsigned char));
		open_offer(from, to, offer);
		buffer_pos += DIPOFFER_UNIT_SIZE;
	}
	*pos = buffer_pos + 1;
	return 0;
}

unsigned int load_game()
{
	/* read file */
	if (!getenv("HOME"))
		return 1;
	struct stat st = { 0 };
	char *savefile = strconcat(getenv("HOME"), SAVE_DIRNAME, SAVE_FILENAME);

	if (stat(savefile, &st) == -1) {
		free(savefile);
		return 2;
	}
	int file_size = st.st_size;

	FILE *fp = fopen(savefile, "r+");
	free(savefile);
	if (fp == NULL)
		return 3;

	char *buffer = malloc(file_size);	/* included nullterm */
	if (!buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	size_t bytes_read = fread(buffer, 1, file_size, fp);
	fclose(fp);
	if (!bytes_read) {
		free(buffer);
		return 4;
	}

	destroy_world();
	create_world();
	/* unpack file data to game structures */
	int pos = 0;
	int (*deserialize[9]) () = {
	&deserialize_game_metadata,
		    &deserialize_characterlist,
		    &deserialize_regionlist,
		    &deserialize_grid,
		    &deserialize_pieces,
		    &deserialize_feudal,
		    &deserialize_heir,
		    &deserialize_diplomacy, &deserialize_dipoffer,};
	int i, result;
	for (i = 0; i < 9; i++) {
		result = deserialize[i] (&buffer, &pos);
		if (result != 0)
			return result;
	}
	free(buffer);
	world->selected_character = get_character_by_id(selected_character_id);
	if (!world->selected_character) return 1;

	sort_region_list();
	sort_diplomacy_list();
	return 0;
}

int serialize_game_metadata(char **buffer)
{
	uint16_t year_be = htobe16(world->current_time.tm_year);
	uint16_t next_character_id_be = htobe16(world->next_character_id);
	uint16_t next_piece_id_be = htobe16(world->next_piece_id);
	uint16_t selected_character_be = (world->selected_character ? htobe16(world->selected_character->id) : 0);
	memcpy(*buffer, &year_be, sizeof(uint16_t));
	memcpy(*buffer + sizeof(uint16_t), &(world->current_time.tm_mon),
	       sizeof(unsigned char));
	memcpy(*buffer + sizeof(uint16_t) + sizeof(unsigned char),
	       &next_character_id_be, sizeof(uint16_t));
	memcpy(*buffer + sizeof(unsigned char) + 2 * sizeof(uint16_t),
	       &next_piece_id_be, sizeof(uint16_t));
	memcpy(*buffer + sizeof(unsigned char) + 3 * sizeof(uint16_t),
	       &selected_character_be, sizeof(uint16_t));
	memcpy(*buffer + sizeof(unsigned char) + 4 * sizeof(uint16_t),
	       &(world->moves_left), sizeof(unsigned char));
	(*buffer)[GAME_METADATA_SIZE] = 'X';
	return GAME_METADATA_SIZE + 1;
}

int serialize_characterlist(char **buffer)
{
	uint16_t nr_characters = count_characters();
	uint16_t nr_characters_be = htobe16(nr_characters);
	int total_len =
	    CHARACTERLIST_METADATA_SIZE + CHARACTERLIST_UNIT_SIZE * nr_characters;
	/** id + name + money + birth_year + birth_mon + death_year + death_mon + heir_id **/
	memcpy(*buffer, &nr_characters_be, CHARACTERLIST_METADATA_SIZE);
	int pos = CHARACTERLIST_METADATA_SIZE;
	character_t *current = world->characterlist;
	uint16_t id_be, money_be, birthdate_year_be, deathdate_year_be;
	int offset = 0;
	while (current != NULL) {
		offset = 0;
		id_be = htobe16(current->id);
		memcpy(*buffer + pos + offset, &id_be, sizeof(uint16_t));	/* id */
		offset += sizeof(uint16_t);
		memcpy(*buffer + pos + offset, current->name,
		       strlen(current->name) + 1);
		offset += 17;
		money_be = htobe16(current->money);
		memcpy(*buffer + pos + offset, &money_be, sizeof(uint16_t));
		offset += sizeof(uint16_t);
		memcpy(*buffer + pos + offset, &(current->rank),
		       sizeof(unsigned char));
		offset += sizeof(unsigned char);
		birthdate_year_be = htobe16(current->birthdate.tm_year);
		memcpy(*buffer + pos + offset, &birthdate_year_be,
		       sizeof(uint16_t));
		offset += sizeof(uint16_t);
		memcpy(*buffer + pos + offset, &(current->birthdate.tm_mon), 1);
		offset += 1;
		deathdate_year_be = htobe16(current->deathdate.tm_year);
		memcpy(*buffer + pos + offset, &deathdate_year_be,
		       sizeof(uint16_t));
		offset += sizeof(uint16_t);
		memcpy(*buffer + pos + offset, &(current->deathdate.tm_mon), 1);
		pos += CHARACTERLIST_UNIT_SIZE;
		current = current->next;
	}
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

int serialize_regionlist(char **buffer)
{
	uint16_t nr_regions = count_regions();
	uint16_t nr_regions_be = htobe16(nr_regions);
	int total_len =
	    REGIONLIST_METADATA_SIZE + REGIONLIST_UNIT_SIZE * nr_regions;
	/* id, name, owner */
	memcpy(*buffer, &nr_regions_be, REGIONLIST_METADATA_SIZE);
	int pos = REGIONLIST_METADATA_SIZE;
	region_t *current = world->regionlist;
	uint16_t id_be, owner_be;
	while (current != NULL) {
		id_be = htobe16(current->id);
		memcpy(*buffer + pos, &id_be, sizeof(uint16_t));	/* id */
/**
		size_be = htobe16(current->size);
		memcpy(*buffer + pos + sizeof(uint16_t), &size_be, sizeof(uint16_t));
**/
		memcpy(*buffer + pos + sizeof(uint16_t), current->name,
		       strlen(current->name) + 1);
		owner_be =
		    (current->owner != NULL ? htobe16(current->owner->id) : 0);
		memcpy(*buffer + pos + sizeof(uint16_t) + 17, &owner_be,
		       sizeof(uint16_t));
		pos += REGIONLIST_UNIT_SIZE;
		current = current->next;
	}
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

int serialize_grid(char **buffer)
{
	uint16_t height_be = htobe16(world->grid->height);
	uint16_t width_be = htobe16(world->grid->width);
	int total_len =
	    GRID_METADATA_SIZE +
	    GRID_UNIT_SIZE * world->grid->height * world->grid->width;
	memcpy(*buffer, &height_be, sizeof(uint16_t));
	memcpy(*buffer + sizeof(uint16_t), &width_be, sizeof(uint16_t));
	int pos = GRID_METADATA_SIZE;
	uint16_t region_be;
	int i, j;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			memcpy(*buffer + pos,
			       &world->grid->tiles[i][j]->walkable, 1);
			region_be =
			    (world->grid->tiles[i][j]->region !=
			     NULL ? htobe16(world->grid->tiles[i][j]->region->
					    id) : 0);
			memcpy(*buffer + pos + 1, &region_be, sizeof(uint16_t));
			pos += GRID_UNIT_SIZE;
		}
	}
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

int serialize_pieces(char **buffer, uint16_t nr_pieces)
{
	int pos = PIECES_METADATA_SIZE;
	uint16_t id_be, height_be, width_be, owner_be;
	int i, j;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			if (world->grid->tiles[i][j]->piece != NULL) {
				id_be =
				    htobe16(world->grid->tiles[i][j]->piece->
					    id);
				memcpy(*buffer + pos, &id_be, sizeof(uint16_t));
				memcpy(*buffer + pos + sizeof(uint16_t),
				       &(world->grid->tiles[i][j]->piece->type),
				       1);
				height_be = htobe16(i);
				memcpy(*buffer + pos + sizeof(uint16_t) + 1,
				       &height_be, sizeof(uint16_t));
				width_be = htobe16(j);
				memcpy(*buffer + pos + sizeof(uint16_t) + 1 +
				       sizeof(uint16_t), &width_be,
				       sizeof(uint16_t));
				owner_be =
				    htobe16(world->grid->tiles[i][j]->piece->
					    owner->id);
				memcpy(*buffer + pos + sizeof(uint16_t) + 1 +
				       2 * sizeof(uint16_t), &owner_be,
				       sizeof(uint16_t));
				pos += PIECES_UNIT_SIZE;
			}
		}
	}
	uint16_t nr_pieces_be = htobe16(nr_pieces);
	memcpy(*buffer, &nr_pieces_be, sizeof(uint16_t));
	int total_len = PIECES_METADATA_SIZE + PIECES_UNIT_SIZE * nr_pieces;
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

int serialize_feudal(char **buffer, uint16_t nr_feudal)
{
	uint16_t nr_feudal_be = htobe16(nr_feudal);
	int total_len = FEUDAL_METADATA_SIZE + FEUDAL_UNIT_SIZE * nr_feudal;
	memcpy(*buffer, &nr_feudal_be, FEUDAL_METADATA_SIZE);
	int pos = FEUDAL_METADATA_SIZE;
	character_t *current = world->characterlist;
	uint16_t vassal_id_be, lord_id_be;
	while (current != NULL) {
		if (current->lord != NULL) {
			vassal_id_be = htobe16(current->id);
			lord_id_be = htobe16(current->lord->id);
			memcpy(*buffer + pos, &vassal_id_be, sizeof(uint16_t));
			memcpy(*buffer + pos + sizeof(uint16_t), &lord_id_be,
			       sizeof(uint16_t));
			pos += FEUDAL_UNIT_SIZE;
		}
		current = current->next;
	}
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

int serialize_heir(char **buffer, uint16_t nr_heir)
{
	uint16_t nr_heir_be = htobe16(nr_heir);
	int total_len = HEIR_METADATA_SIZE + HEIR_UNIT_SIZE * nr_heir;
	memcpy(*buffer, &nr_heir_be, HEIR_METADATA_SIZE);
	int pos = HEIR_METADATA_SIZE;
	character_t *current = world->characterlist;
	uint16_t character_id_be, heir_id_be;
	while (current != NULL) {
		if (current->heir != NULL) {
			character_id_be = htobe16(current->id);
			heir_id_be = htobe16(current->heir->id);
			memcpy(*buffer + pos, &character_id_be, sizeof(uint16_t));
			memcpy(*buffer + pos + sizeof(uint16_t), &heir_id_be,
			       sizeof(uint16_t));
			pos += HEIR_UNIT_SIZE;
		}
		current = current->next;
	}
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

int serialize_diplomacy(char **buffer, uint16_t nr_dipstat)
{
	uint16_t nr_dipstat_be = htobe16(nr_dipstat);
	int total_len =
	    DIPLOMACY_METADATA_SIZE + DIPLOMACY_UNIT_SIZE * nr_dipstat;
	memcpy(*buffer, &nr_dipstat_be, DIPLOMACY_METADATA_SIZE);
	int pos = DIPLOMACY_METADATA_SIZE;
	dipstatus_t *current = world->diplomacylist;
	uint16_t character1_id_be, character2_id_be;
	while (current != NULL) {
		character1_id_be = htobe16(current->character1->id);
		character2_id_be = htobe16(current->character2->id);
		memcpy(*buffer + pos, &character1_id_be, sizeof(uint16_t));
		memcpy(*buffer + pos + sizeof(uint16_t), &character2_id_be,
		       sizeof(uint16_t));
		memcpy(*buffer + pos + sizeof(uint16_t) * 2, &(current->status),
		       sizeof(unsigned char));
		pos += DIPLOMACY_UNIT_SIZE;
		current = current->next;
	}
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

int serialize_dipoffer(char **buffer, uint16_t nr_dipoffer)
{
	uint16_t nr_dipoffer_be = htobe16(nr_dipoffer);
	int total_len =
	    DIPOFFER_METADATA_SIZE + DIPOFFER_UNIT_SIZE * nr_dipoffer;
	memcpy(*buffer, &nr_dipoffer_be, DIPOFFER_METADATA_SIZE);
	int pos = DIPOFFER_METADATA_SIZE;
	dipstatus_t *current = world->diplomacylist;
	uint16_t from_id_be, to_id_be;
	while (current != NULL) {
		if (current->pending_offer != NULL) {
			from_id_be = htobe16(current->pending_offer->from->id);
			to_id_be = htobe16(current->pending_offer->to->id);
			memcpy(*buffer + pos, &from_id_be, sizeof(uint16_t));
			memcpy(*buffer + pos + sizeof(uint16_t), &to_id_be,
			       sizeof(uint16_t));
			memcpy(*buffer + pos + sizeof(uint16_t) * 2,
			       &(current->pending_offer->offer),
			       sizeof(unsigned char));
			pos += DIPOFFER_UNIT_SIZE;
		}
		current = current->next;
	}
	(*buffer)[total_len] = 'X';
	return total_len + 1;
}

unsigned int save_game()
{
	unsigned int retval = 0;
	/* pack data */
	if (!getenv("HOME"))
		return 1;
	/* write to file */
	char *savefile = strconcat(getenv("HOME"), SAVE_DIRNAME, SAVE_FILENAME);
	FILE *fp = fopen(savefile, "w");
	free(savefile);
	if (!fp) {
		retval = 1;
		goto ret;
	}

	int bytes = 0;
	size_t bytes_written;

	char *game_metadata_buffer = calloc(1, GAME_METADATA_SIZE + 1);
	if (!game_metadata_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_game_metadata(&game_metadata_buffer);
	if (!bytes) {
		fclose(fp);
		retval = 3;
		goto ret;
	}
	bytes_written = fwrite(game_metadata_buffer, sizeof(char), bytes, fp);
	free(game_metadata_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 4;
		goto ret;
	}

	char *characterlist_buffer =
	    calloc(1,
		   CHARACTERLIST_METADATA_SIZE +
		   count_characters() * CHARACTERLIST_UNIT_SIZE + 1);
	if (!characterlist_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_characterlist(&characterlist_buffer);
	if (!bytes) {
		fclose(fp);
		retval = 3;
		goto ret;
	}
	bytes_written = fwrite(characterlist_buffer, sizeof(char), bytes, fp);
	free(characterlist_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 4;
		goto ret;
	}

	char *regionlist_buffer =
	    calloc(1,
		   REGIONLIST_METADATA_SIZE +
		   count_regions() * REGIONLIST_UNIT_SIZE + 1);
	if (!regionlist_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_regionlist(&regionlist_buffer);
	if (!bytes) {
		fclose(fp);
		retval = 3;
		goto ret;
	}
	bytes_written = fwrite(regionlist_buffer, sizeof(char), bytes, fp);
	free(regionlist_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 4;
		goto ret;
	}

	char *grid_buffer =
	    calloc(1,
		   GRID_METADATA_SIZE +
		   world->grid->height * world->grid->width * GRID_UNIT_SIZE +
		   1);
	if (!grid_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_grid(&grid_buffer);
	if (!bytes) {
		fclose(fp);
		retval = 6;
		goto ret;
	}
	bytes_written = fwrite(grid_buffer, sizeof(char), bytes, fp);
	free(grid_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 7;
		goto ret;
	}

	uint16_t nr_pieces = count_pieces();
	char *pieces_buffer =
	    calloc(1, PIECES_METADATA_SIZE + nr_pieces * PIECES_UNIT_SIZE + 1);
	if (!pieces_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_pieces(&pieces_buffer, nr_pieces);
	if (!bytes) {
		fclose(fp);
		retval = 9;
		goto ret;
	}
	bytes_written = fwrite(pieces_buffer, sizeof(char), bytes, fp);
	free(pieces_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 10;
		goto ret;
	}

	int nr_feudal = 0;
	character_t *current = world->characterlist;
	while (current != NULL) {
		if (current->lord != NULL)
			nr_feudal++;
		current = current->next;
	}
	char *feudal_buffer =
	    calloc(1, FEUDAL_METADATA_SIZE + nr_feudal * FEUDAL_UNIT_SIZE + 1);
	if (!feudal_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_feudal(&feudal_buffer, nr_feudal);
	if (!bytes) {
		fclose(fp);
		retval = 12;
		goto ret;
	}
	bytes_written = fwrite(feudal_buffer, sizeof(char), bytes, fp);
	free(feudal_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 13;
		goto ret;
	}

	int nr_heir = 0;
	current = world->characterlist;
	while (current != NULL) {
		if (current->heir != NULL)
			nr_heir++;
		current = current->next;
	}
	char *heir_buffer =
	    calloc(1, HEIR_METADATA_SIZE + nr_heir * HEIR_UNIT_SIZE + 1);
	if (!heir_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_heir(&heir_buffer, nr_heir);
	if (!bytes) {
		fclose(fp);
		retval = 15;
		goto ret;
	}
	bytes_written = fwrite(heir_buffer, sizeof(char), bytes, fp);
	free(heir_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 16;
		goto ret;
	}

	int nr_dipstat = 0;
	remove_redundant_diplomacy();
	dipstatus_t *current_diplomacy = world->diplomacylist;
	while (current_diplomacy != NULL) {
		nr_dipstat++;
		current_diplomacy = current_diplomacy->next;
	}
	char *diplomacy_buffer =
	    calloc(1,
		   DIPLOMACY_METADATA_SIZE + nr_dipstat * DIPLOMACY_UNIT_SIZE +
		   1);
	if (!diplomacy_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_diplomacy(&diplomacy_buffer, nr_dipstat);
	if (!bytes) {
		fclose(fp);
		retval = 18;
		goto ret;
	}
	bytes_written = fwrite(diplomacy_buffer, sizeof(char), bytes, fp);
	free(diplomacy_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 19;
		goto ret;
	}

	int nr_dipoffer = 0;
	current_diplomacy = world->diplomacylist;
	while (current_diplomacy != NULL) {
		if (current_diplomacy->pending_offer != NULL)
			nr_dipoffer++;
		current_diplomacy = current_diplomacy->next;
	}
	char *dipoffer_buffer =
	    calloc(1,
		   DIPOFFER_METADATA_SIZE + nr_dipoffer * DIPOFFER_UNIT_SIZE +
		   1);
	if (!dipoffer_buffer) {
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bytes = serialize_dipoffer(&dipoffer_buffer, nr_dipoffer);
	if (!bytes) {
		fclose(fp);
		retval = 21;
		goto ret;
	}
	bytes_written = fwrite(dipoffer_buffer, sizeof(char), bytes, fp);
	free(dipoffer_buffer);
	if (!bytes_written) {
		fclose(fp);
		retval = 22;
		goto ret;
	}

	fclose(fp);
 ret:
	return retval;
}

char **load_namelist(const char *filename, const char *placeholder, const int size) {
	/**
	 * First, search for filename in ~/.feud.
	 * If not found, search in DATADIR.
	 * If not found, use numbered naming
	**/
	char *namelist_file[3];
	namelist_file[0] = strconcat(getenv("HOME"), SAVE_DIRNAME, "/", filename);
	namelist_file[1] = strconcat(DATADIR, "/", filename);
	namelist_file[2] = malloc(sizeof(char) * strlen(filename) + 1);
	if (!namelist_file[2]) exit(EXIT_FAILURE);
	strncpy(namelist_file[2], filename, strlen(filename));
	int i = 0;
	int retval;
	for (i = 0; i < 3; i++) {
		if (!namelist_file[i]) continue;
		retval = access(namelist_file[i], F_OK|R_OK);
		if (retval != -1) break;
	}
	int current_line = 0;
	char **namelist = malloc(sizeof(char *) * size);
	if (!namelist) exit(EXIT_FAILURE);
	FILE *fp = fopen(namelist_file[i], "r+");
	if (!fp) {
		free(namelist);
		return NULL;
	}
	/**
	 * read first char
	 * if space, skip
	 * if #, fast-forward to next line
	 * if empty line, ff to next line
	 * else read 16 chars and ff to next line
	**/
	int current_char = 0;
	int current_char_nr = 0;
	int fast_forward = 0;
	namelist[0] = malloc(17);
	if (!namelist[0]) exit(EXIT_FAILURE);
	while (current_line < size && !feof(fp)) {
		current_char = fgetc(fp);
		if (fast_forward == 1) {
			if ((char) current_char == '\n') {
				fast_forward = 0;
				current_char_nr = 0;
				continue;
			}
		}
		else {
			if (current_char_nr == 0) {
				if ((char) current_char == '#') {
					fast_forward = 1;
					continue;
				}
				if ((char) current_char != '\n' && (char) current_char != '\r') {
					namelist[current_line] = malloc(sizeof(char) * 17);
					if (!namelist[current_line]) exit(EXIT_FAILURE);
					namelist[current_line][0] = (char) current_char;
					current_char_nr++;
					continue;
				}
			}
			else if (current_char_nr < 16) {
				if ((char) current_char == '\n' || (char) current_char == '\r') {
					namelist[current_line][current_char_nr] = '\0';
					current_line++;
					current_char_nr = 0;
					continue;
				}
				else {
					strncpy(namelist[current_line]+current_char_nr, (char *) &current_char, 1);
					current_char_nr++;
					continue;
				}
			}
			else {
				namelist[current_line][current_char_nr] = '\0';
				current_char_nr = 0;
				fast_forward = 1;
				current_line++;
			}
		}
	}
	fclose(fp);
	if (current_line < size) {
		int digits = floor(log10(abs(size))) + 1;
		for (i = current_line; i < size; i++) {
			namelist[i] = malloc(sizeof(char) * 17);
			if (!namelist[i]) exit(EXIT_FAILURE);
			sprintf(namelist[i], "%s%0*d", placeholder, digits, i + 1);
		}
	}
	return namelist;
}

int savefile_exists() {
	int retval = 0;
	if (!getenv("HOME")) return 0;
	char *dirname = strconcat(getenv("HOME"), SAVE_DIRNAME);
	struct stat st = { 0 };
	if (stat(dirname, &st) == -1) {
		mkdir(dirname, 0700);
	}
	char *filename = strconcat(dirname, SAVE_FILENAME);
	free(dirname);
	if (access(filename, F_OK) != -1) retval = 1;
	free(filename);
	return retval;
}

void delete_savefile() {
	if (!getenv("HOME")) return;
	char *filename = strconcat(getenv("HOME"), SAVE_DIRNAME, SAVE_FILENAME);
	if (access(filename, F_OK) == 0) remove(filename);
	free(filename);
}

void rename_logfile() {
	if (!getenv("HOME")) return;
	char *old_filename = strconcat(getenv("HOME"), SAVE_DIRNAME, LOG_FILENAME);
	char unixtime[100];
//	time_t *now;
	time_t time_now = time(NULL);
	snprintf(unixtime, 99, "%i", (int) time_now);
	char *new_filename = strconcat(getenv("HOME"), SAVE_DIRNAME, "/", unixtime, ".log");
	rename(old_filename, new_filename);
	free(old_filename);
	free(new_filename);
}
