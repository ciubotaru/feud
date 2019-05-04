#ifndef PIECE_H
#define PIECE_H

#include "character.h"

enum piece_type {NOBLE, SOLDIER, SHIP};

#define COST_SOLDIER 1

typedef struct piece {
	uint16_t id;
	unsigned char type;
	struct tile *tile;
	character_t *owner;
	struct piece *next;
} piece_t;

extern char *const piece_name[];

piece_t *add_piece(const enum piece_type type, const uint16_t height,
		   const uint16_t width, character_t *owner);

piece_t *get_noble_by_owner(character_t *owner);

piece_t *next_piece(piece_t *start_piece);

void remove_piece(piece_t *piece);

void clear_piece_list();

uint16_t count_pieces();

uint16_t count_pieces_by_owner(character_t *owner);

#endif				/* PIECE_H */
