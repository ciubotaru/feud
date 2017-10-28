#include <stdlib.h>
#include <stdio.h>
#include "dice.h"
#include "world.h"

unsigned char get_dice() {
	return (rand() % 6) + 1; 
}
