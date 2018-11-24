#ifndef PIECE_H
#define PIECE_H

#include "character.h"

#define NOBLE 0
#define SOLDIER 1
//#define SHIP 2
#define COST_SOLDIER 1

typedef struct piece {
	uint16_t id;
	unsigned char type;
	struct tile *tile;
	character_t *owner;
	struct piece *next;
} piece_t;

char *const unit_type_list[3];

piece_t *add_piece(const unsigned char type, const uint16_t height,
		   const uint16_t width, character_t *owner);

piece_t *get_noble_by_owner(character_t *owner);

piece_t *next_piece(piece_t *start_piece);

void remove_piece(piece_t *piece);

void clear_piece_list();

uint16_t count_pieces();

uint16_t count_pieces_by_owner(character_t *owner);

void update_army_ranking();

#endif				/* PIECE_H */
