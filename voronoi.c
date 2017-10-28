#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "map.h"
#include "voronoi.h"
#include "world.h"

int region_size = 36;

int voronoi_iterations = 3;

float **region_centers = NULL;

float **create_region_centers(uint16_t nr_regions)
{
	if (world->grid == NULL || nr_regions == 0)
		return NULL;
	int i, j, tmp;
	int numbers[world->grid->height * world->grid->width];
	for (i = 0; i < world->grid->height * world->grid->width; i++)
		numbers[i] = i;
	for (i = 0; i < world->grid->height * world->grid->width / 4; i++) {
		j = i +
		    rand() / (RAND_MAX /
			      (world->grid->height * world->grid->width - i) +
			      1);
		tmp = numbers[j];
		numbers[j] = numbers[i];
		numbers[i] = tmp;
	}
	region_centers = malloc(sizeof(float *) * nr_regions);
	for (i = 0; i < nr_regions; i++) {
		region_centers[i] = malloc(sizeof(float) * 2);
		region_centers[i][0] = numbers[i] / world->grid->width;
		region_centers[i][1] = numbers[i] % world->grid->width;
	}
	return region_centers;
}

void create_regions(uint16_t nr_regions)
{
	if (world->regionlist != NULL || nr_regions == 0)
		return;
	int i;
//      region_t *region = NULL;
	char *name = malloc(floor(log10(nr_regions)) + 2);
	for (i = 0; i < nr_regions; i++) {
		sprintf(name, "%03d", i);
//              region = add_region(name);
		add_region(name);
	}
	free(name);
}

void assign_tiles_to_centers2()
{
	uint16_t nr_regions = count_regions();
	if (world->grid == NULL || nr_regions == 0 || region_centers == NULL)
		return;
	int i, j, k;
	int closest_center = 0;
	float min_distance = 0;
	float tmp;
	region_t *z = NULL;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			closest_center = 0;
			min_distance =
			    (float)
			    sqrt(powf
				 (((float)i + 0.5 -
				   region_centers[closest_center][0]),
				  2) + powf(((float)j + 0.5 -
					     region_centers[closest_center][1]),
					    2));
			for (k = 1; k < nr_regions; k++) {
				tmp =
				    (float)
				    sqrt(powf
					 (((float)i + 0.5 -
					   region_centers[k][0]),
					  2) + powf(((float)j + 0.5 -
						     region_centers[k][1]), 2));
				if (tmp < min_distance) {
					closest_center = k;
					min_distance = tmp;
				}
			}
			z = get_region_by_id(closest_center + 1);
//printf("Assigning tile %i, %i to region %s.\n", i, j, z->name);
			change_tile_region(z, world->grid->tiles[i][j]);
		}
	}
}

void assign_tiles_to_centers()
{
	uint16_t nr_regions = count_regions();
	if (world->grid == NULL || nr_regions == 0 || region_centers == NULL)
		return;
	int i, j, counter;
	int closest_center;
	float min_distance = 0;
	float tmp;
	region_t *z = NULL;
	region_t *current = NULL;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			closest_center = world->regionlist->id;
			min_distance =
			    (float)
			    sqrt(powf
				 (((float)i + 0.5 -
				   region_centers[closest_center][0]),
				  2) + powf(((float)j + 0.5 -
					     region_centers[closest_center][1]),
					    2));
			counter = 1;
			current = world->regionlist->next;
			while (current != NULL) {
//                      for (k = 1; k < nr_regions; k++) {
				tmp =
				    (float)
				    sqrt(powf
					 (((float)i + 0.5 -
					   region_centers[counter][0]),
					  2) + powf(((float)j + 0.5 -
						     region_centers[counter]
						     [1]), 2));
				if (tmp < min_distance) {
					closest_center = current->id;
					min_distance = tmp;
				}
				counter++;
				current = current->next;
			}
			z = get_region_by_id(closest_center);
			change_tile_region(z, world->grid->tiles[i][j]);
		}
	}
}

void recalculate_region_centers2()
{
	int i, j, k;
	int cumul_h = 0, cumul_w = 0;
	region_t *region = NULL;
	uint16_t nr_regions = count_regions();
	for (k = 0; k < nr_regions; k++) {
		cumul_h = 0;
		cumul_w = 0;
		region = get_region_by_id(k + 1);
		for (i = 0; i < world->grid->height; i++) {
			for (j = 0; j < world->grid->width; j++) {
				if (world->grid->tiles[i][j]->region->id ==
				    k + 1) {
					cumul_h += i;
					cumul_w += j;
				}
			}
		}
		region_centers[k][0] = (float)cumul_h / region->size;
		region_centers[k][1] = (float)cumul_w / region->size;
	}
}

void recalculate_region_centers()
{
	int i, j;
	int cumul_h = 0, cumul_w = 0;
//      uint16_t nr_regions = count_regions();
	int counter = 0;
	region_t *current = world->regionlist;
	while (current != NULL) {
//      while (counter < nr_regions) {
		cumul_h = 0;
		cumul_w = 0;
		for (i = 0; i < world->grid->height; i++) {
			for (j = 0; j < world->grid->width; j++) {
				if (world->grid->tiles[i][j]->region->id ==
				    current->id) {
					cumul_h += i;
					cumul_w += j;
				}
			}
		}
		region_centers[counter][0] = (float)cumul_h / current->size;
		region_centers[counter][1] = (float)cumul_w / current->size;
		current = current->next;
		counter++;
	}
}

void voronoi()
{
	if (world->grid == NULL)
		return;
	int i;
	uint16_t nr_regions =
	    (world->grid->height) * (world->grid->width) / region_size;
	region_centers = create_region_centers(nr_regions);
	create_regions(nr_regions);
	assign_tiles_to_centers2();
	for (i = 0; i < voronoi_iterations; i++) {
		recalculate_region_centers();
		assign_tiles_to_centers2();
	}
	for (i = 0; i < nr_regions; i++)
		free(region_centers[i]);
	free(region_centers);
}
