/**
* set average region size (e.g. 36)
* count tiles (e.g. 24x100 = 2400) and determine the number of regions (e.g. 2400 / 36 = 67)
* randomly assign region centers to float (do not overlap within same tile/integer)
* create regions (name = order number)
repeat X times
* for each tile, find closest region center and assign it to that center
* from tiles belonging to a region, recalculate region centers
end
* distort borders ?
**/
#ifndef MAPGEN_H
#define MAPGEN_H
#include <stdint.h>

extern int voronoi_iterations;

extern float **region_centers;

float **create_region_centers(uint16_t nr_regions);

void create_regions(uint16_t nr_regions);

void create_characters(uint16_t nr_characters);

void assign_tiles_to_centers();

void voronoi(int nr_regions);

unsigned char **create_height_grid();

void delete_height_grid(unsigned char **grid);

void populate_height_grid(unsigned char **grid);

void blur_height_grid(unsigned char **grid);

int create_contiguous_area(unsigned char **grid, unsigned int percent_unwalkable);

#endif				/* MAPGEN_H */
