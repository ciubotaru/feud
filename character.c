#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"
#include "world.h"

char *const rank_name[] = {
	"knight",
	"baron",
	"count",
	"duke",
	"king"
};

inline static void set_expected_age(character_t *character);

character_t *create_characterlist()
{
	/* single instance */
	if (world->characterlist != NULL)
		return world->characterlist;
	world->characterlist = malloc(sizeof(character_t));
	if (world->characterlist == NULL) exit(EXIT_FAILURE);
	return world->characterlist;
}

static void fill_character_details(character_t *character, const char *name)
{
	if (!character) return;
	character->id = world->next_character_id;
	world->next_character_id++;
	strcpy(character->name, name);
	character->money = 0;
	character->rank = 0;
	character->prev = NULL;
	character->next = NULL;
	character->birthdate.tm_year = world->current_time.tm_year;
	character->birthdate.tm_mon = world->current_time.tm_mon;
	character->lord = NULL;
	character->heir = NULL;
	set_expected_age(character);
}

character_t *add_character(const char *name)
{
	if (world->characterlist == NULL) {
		world->characterlist = create_characterlist();
		fill_character_details(world->characterlist, name);
		return world->characterlist;
	}

	character_t *current = world->characterlist;

	/*fast-forward to the end of list */
	while (current->next != NULL) {
		current = current->next;
	}

	/* now we can add a new variable */
	current->next = malloc(sizeof(character_t));
	if (!current->next) exit(EXIT_FAILURE);
	fill_character_details(current->next, name);
	current->next->prev = current;
	return current->next;
}

character_t *add_character_before(character_t *parent, const char *name)
{
	if (world->characterlist == NULL) {
		world->characterlist = create_characterlist();
		fill_character_details(world->characterlist, name);
		return world->characterlist;
	}

	character_t *current = world->characterlist;

	/*fast-forward to parent or to the end of list*/
	while (current != parent && current->next != NULL) {
		current = current->next;
	}

	/* now we can add a new variable */
	character_t *new = malloc(sizeof(character_t));
	if (!new) exit(EXIT_FAILURE);
	fill_character_details(new, name);

	if (current->prev) {
		current->prev->next = new;
		new->prev = current->prev;
	}
	else world->characterlist = new;
	new->next = current;
	current->prev = new;
	return new;
}

void remove_character(character_t *character)
{
	if (character == NULL)
		return;

	/* remove all pieces that belong to this character */
	piece_t *current_piece = world->piecelist;
	piece_t *previous_piece = NULL;
	while (current_piece != NULL) {
		previous_piece = current_piece;
		current_piece = current_piece->next;
		if (previous_piece->owner->id == character->id) {
			remove_piece(previous_piece);
		}
	}

	/* free all regions that belong to this character */
	region_t *current_region = world->regionlist;
	while (current_region != NULL) {
		if (current_region->owner != NULL
		    && current_region->owner->id == character->id) {
			current_region->owner = NULL;
		}
		current_region = current_region->next;
	}
	/* remove all diplomacy that included this character */
	dipstatus_t *dipstatus = world->diplomacylist;
	while (dipstatus != NULL) {
		if (dipstatus->character1 == character
		    || dipstatus->character2 == character) {
			dipstatus_t *tmp = dipstatus->next;
			remove_diplomacy(dipstatus);
			dipstatus = tmp;
		} else
			dipstatus = dipstatus->next;
	}

	/* if this character was set as heir or lord to somebody else, remove him */
	character_t *current = world->characterlist;
	while (current != NULL) {
		if (current->heir != NULL && current->heir->id == character->id)
			current->heir = NULL;
		if (current->lord != NULL && current->lord->id == character->id)
			current->lord = NULL;
		current = current->next;
	}

	if (character == world->selected_character) {
		if (character->next != NULL) world->selected_character = character->next;
		else if (character != world->characterlist) world->selected_character = world->characterlist;
		else world->selected_character = NULL;
	}

//	character_t *prev = NULL;
	current = world->characterlist;
	while (current != NULL) {
		if (current == character) {
			if (current->prev) current->prev->next = current->next;
			else world->characterlist = current->next;
			if (current->next) current->next->prev = current->prev;
			character_t *tmp = current;
			free(tmp);
			return;
		}
//		prev = current;
		current = current->next;
	}
}

void clear_character_list()
{
	while (world->characterlist != NULL)
		remove_character(world->characterlist);
	world->next_character_id = 1;
}

uint16_t get_money(character_t *character)
{
	/* can be accessed without a function */
	return character->money;
}

void set_money(character_t *character, const uint16_t money)
{
	if (character == NULL)
		return;
	if (money > MONEY_MAX) character->money = MONEY_MAX;
	else character->money = money;
	return;
}

character_t *get_character_by_name(const char *name)
{
	character_t *current = world->characterlist;
	while (current != NULL) {
		if (strcmp(current->name, name) == 0) {
			return current;
		}
		if (current->next == NULL)
			return NULL;
		current = current->next;
	}
	return NULL;
}

character_t *get_character_by_id(const uint16_t id)
{
	character_t *current = world->characterlist;
	while (current != NULL) {
		if (id == current->id) {
			return current;
		}
		if (current->next == NULL)
			return NULL;
		current = current->next;
	}
	return NULL;
}

uint16_t get_character_order(character_t *character)
{
	if (character == NULL)
		return 0;
	uint16_t counter = 0;
	character_t *current = world->characterlist;
	while (current != NULL && current->id != character->id) {
		counter++;
		current = current->next;
	}
	return counter;
}

unsigned char get_character_rank(character_t *character)
{
	if (character == NULL)
		return 0;
	return character->rank;
}

void set_character_rank(character_t *character, unsigned char rank)
{
	if (character == NULL)
		return;
	character->rank = rank;
}

int transfer_money(character_t *source, character_t *destination, const int amount)
{
	/* check if source and destination exist. NEEDED? */
	if (source == NULL || destination == NULL
	    || source->id == destination->id)
		return 1;
	/* check if source has enough */
	if (source->money < amount)
		return 1;
	/* check if destination will not exceed the limit */
	if (destination->money + amount > MONEY_MAX) return 1;
	source->money -= amount;
	destination->money += amount;
	add_to_chronicle("%s granted %i coins to %s.\n", source->name, amount, destination->name);
	return 0;
}

inline static void set_expected_age(character_t *character)
{
	unsigned int months =
	    (character->birthdate.tm_year + MIN_AGE) * 12 +
	    character->birthdate.tm_mon;
	months += rand() % ((MAX_AGE - MIN_AGE) * 12);
	character->deathdate.tm_year = months / 12;
	character->deathdate.tm_mon = months % 12;
}

void set_successor(character_t *character, character_t *successor)
{
	if (character == NULL
	    || (successor != NULL && character->id == successor->id))
		return;		/* can not name yourself a heir */
	character->heir = successor;
	return;
}

uint16_t count_characters()
{
	uint16_t count = 0;
	character_t *current = world->characterlist;
	while (current != NULL) {
		count++;
		current = current->next;
	}
	return count;
}

int is_gameover()
{
	/**
	 * The game is over if one character, or one sovereign with his vassals,
	 * is left
	**/
	character_t *character = NULL;
	int nr_sovereigns = 0;
	character_t *current = world->characterlist;
	while (current != NULL) {
		if (current->lord == NULL)
			nr_sovereigns++;
		current = current->next;
	}
	if (nr_sovereigns == 1) {
		character = get_sovereign(world->characterlist);
		add_to_chronicle
		    ("%s is the only sovereign left. The game is over!\n",
		     character->name);
		return 1;
	}
	return 0;
}

void check_death()
{
	world->message[0] = '\0';
	if (world->check_death == 0)
		return;
	/**
	 * loop through characterlist, check if every character has a noble and
	 * at least one region
	**/
	character_t *current_character = world->characterlist;
	character_t *next = NULL;
	while (current_character != NULL) {
		next = current_character->next;
		/* if a character loses all his land, he loses the game */
		if (count_tiles_by_owner(current_character) == 0) {
			add_to_chronicle("%s %s lost all his land.\n",
					rank_name[current_character->rank],
					current_character->name);
			add_to_chronicle(world->message);
			succession(current_character);
			remove_character(current_character);
		}
		/* if a character loses his noble, he loses the game */
		else if (get_noble_by_owner(current_character) == NULL) {
			add_to_chronicle("%s %s was killed.\n",
					rank_name[current_character->rank],
					current_character->name);
			add_to_chronicle(world->message);
			succession(current_character);
			remove_character(current_character);
		}
		/* if a character reaches old age, he loses the game */
		else if (world->current_time.tm_year +
			 12 * world->current_time.tm_mon >=
			 current_character->deathdate.tm_year +
			 12 * current_character->deathdate.tm_mon) {
			add_to_chronicle("%s %s died of old age.\n",
					rank_name[current_character->rank],
					current_character->name);
			add_to_chronicle(world->message);
			succession(current_character);
			remove_character(current_character);
		}
		current_character = next;
	}
	world->check_death = 0;
}

void succession(character_t *character)
{
	/**
	 * First, find the successor.
	 * If character named a heir, the heir will succeed.
	 * If the character did not name a heir, but has a lord, the lord will succeed.
	 * If the character did not name a heir and has no lord, the property is lost.
	 **/
	character_t *heir = NULL;
	if (character->heir != NULL) {
		heir = character->heir;
		add_to_chronicle("%s %s inherited the property of %s %s.\n",
				rank_name[heir->rank], heir->name,
				rank_name[character->rank], character->name);
	} else if (character->lord != NULL) {
		heir = character->lord;
		add_to_chronicle
		    ("%s %s had not named a heir, and their lord, %s %s, inherited everything.\n",
		     rank_name[character->rank], character->name, rank_name[heir->rank],
		     heir->name);
	} else {
		add_to_chronicle("%s %s had no one to inherit their property.\n",
				rank_name[character->rank], character->name);
		return;
	}

	/* the heir always inherits money */
	if (heir->money + character->money > MONEY_MAX) heir->money = MONEY_MAX;
	else heir->money += character->money;

	/* the heir always inherits land */
	region_t *current_region = world->regionlist;
	while (current_region != NULL) {
		if (current_region->owner != NULL
		    && current_region->owner->id == character->id)
			current_region->owner = heir;
		current_region = current_region->next;
	}

	/* the heir always inherits soldiers */
	piece_t *current_piece = world->piecelist;
	while (current_piece != NULL) {
		if (current_piece->owner->id == character->id
		    && current_piece->type == 1)
			current_piece->owner = heir;
		current_piece = current_piece->next;
	}

	/* the heir inherits grantor's title, if it's higher than his own */
	if (character->rank > heir->rank) {
		add_to_chronicle("%s inherited the %s title.\n", heir->name,
				rank_name[character->rank]);
		heir->rank = character->rank;
	}

	/* the heir always inherits grantor's vassals */
	character_t *vassal = world->characterlist;
	while (vassal != NULL) {
		if (vassal->lord != NULL && vassal->lord->id == character->id) {
			vassal->lord = heir;
			set_diplomacy(vassal, character, ALLIANCE);
		}
		vassal = vassal->next;
	}

	/* if the heir has no lord, he does not inherit grantor's lord allegiance */
	if (heir->lord == NULL)
		return;

	/* if possible, the heir keeps his lord allegiance */
//      piece_t *heir_lord_noble = get_noble_by_owner(heir->lord);
	if (character->rank < heir->lord->rank)
		return;
	/* only if character's new title is incompatible with current lord allegiance, he has to change allegiance */
	else {
		/* if grantor had no lord, the heir becomes free */
		if (character->lord == NULL)
			heir->lord = NULL;
		else {
			add_to_chronicle
			    ("%s %s paid homage to %s %s for their title.\n",
			     rank_name[heir->rank], heir->name,
			     rank_name[character->lord->rank], character->lord->name);
			homage(heir, character->lord);
		}
	}
}
