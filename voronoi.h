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
#ifndef VORONOI_H
#define VORONOI_H
#include <stdint.h>

int voronoi_iterations;

float **region_centers;

float **create_region_centers(uint16_t nr_regions);

void create_regions(uint16_t nr_regions);

void assign_tiles_to_centers();

void recalculate_region_centers();

void voronoi();

#endif /* VORONOI_H */
