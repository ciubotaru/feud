#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "world.h"

int region_size = 36;

int voronoi_iterations = 3;

float **region_centers = NULL;

inline static void recalculate_region_centers();

float **create_region_centers(uint16_t nr_regions)
{
	if (world->grid == NULL || nr_regions == 0)
		return NULL;
	int i;
	region_centers = malloc(sizeof(float *) * nr_regions);
	if (!region_centers)  exit(EXIT_FAILURE);
	for (i = 0; i < nr_regions; i++) {
		region_centers[i] = malloc(sizeof(float) * 2);
		if (!region_centers[i]) exit(EXIT_FAILURE);
		region_centers[i][0] = fmod(rand(), world->grid->height);
		region_centers[i][1] = fmod(rand(), world->grid->width);
	}
	return region_centers;
}

void create_regions(uint16_t nr_regions)
{
	if (world->regionlist != NULL || nr_regions == 0)
		return;
	char **region_names_new = load_namelist("regions.txt", "Region", nr_regions);
	int i;
	for (i = 0; i < nr_regions; i++) {
		add_region(region_names_new[i]);
		free(region_names_new[i]);
	}
	free(region_names_new);
}

void create_characters(uint16_t nr_characters)
{
	if (world->characterlist != NULL || nr_characters == 0)
		return;
	char **character_names_new = load_namelist("characters.txt", "Player", nr_characters);
	int i;
	for (i = 0; i < nr_characters; i++) {
		add_character(character_names_new[i]);
		free(character_names_new[i]);
	}
	free(character_names_new);
}

void assign_tiles_to_centers()
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
			if (world->grid->tiles[i][j]->walkable == 0) continue;
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

inline static void recalculate_region_centers()
{
	int i, j;
	int cumul_h = 0, cumul_w = 0;
//      uint16_t nr_regions = count_regions();
	uint16_t counter = 0;
	region_t *current = world->regionlist;
	while (current != NULL) {
//      while (counter < nr_regions) {
		cumul_h = 0;
		cumul_w = 0;
		for (i = 0; i < world->grid->height; i++) {
			for (j = 0; j < world->grid->width; j++) {
				if (world->grid->tiles[i][j]->region && world->grid->tiles[i][j]->region->id ==
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

void voronoi(int nr_regions)
{
	if (world->grid == NULL)
		return;
	int i;
	region_centers = create_region_centers(nr_regions);
	assign_tiles_to_centers();
	for (i = 0; i < voronoi_iterations; i++) {
		recalculate_region_centers();
		assign_tiles_to_centers();
	}
	for (i = 0; i < nr_regions; i++)
		free(region_centers[i]);
	free(region_centers);
}

unsigned char **create_height_grid() {
	if (!world->grid || !world->grid->tiles) return NULL;
	unsigned char **grid = malloc(sizeof(char *) * world->grid->height);
	if (!grid)  exit(EXIT_FAILURE);
	int i;
	for (i = 0; i < world->grid->height; i++) {
		grid[i] = malloc(world->grid->width);
		if (!grid[i])  exit(EXIT_FAILURE);
	}
	return grid;
}

void delete_height_grid(unsigned char **grid) {
	if (!grid) return;
	int i;
	for (i = 0; i < world->grid->height; i++) {
		free(grid[i]);
	}
	free(grid);
}

void populate_height_grid(unsigned char **grid) {
	if (!world->grid || !world->grid->tiles) return;
	int i, j;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			grid[i][j] = rand() % 256;
		}
	}
}

void blur_height_grid(unsigned char **grid) {
	if (!world->grid || !world->grid->tiles) return;
	int i, j, x, y, x_min, x_max, y_min, y_max, count;
	unsigned char **grid_tmp = create_height_grid();
	if (!grid_tmp) return;
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			grid_tmp[i][j] = grid[i][j];
		}
	}
	int radius = 7;
	for(i = 0; i < world->grid->height; i++) {
		x_min = 0 > i - radius ? 0 : i - radius;
		x_max = world->grid->height < i + radius ? world->grid->height : i + radius;
		for(j = 0; j < world->grid->width; j++) {
			y_min = 0 > j - radius ? 0 : j - radius;
			y_max = world->grid->width < j + radius ? world->grid->width : j + radius;
			int sum = 0;
			count = 0;
			for(x = x_min; x < x_max; x++){
				for(y = y_min; y < y_max; y++){
//					if ((x - i)^2 + (y - j)^2 <= radius^2) {
						sum += grid_tmp[x][y];
						count++;
//					}
				}
			}
			grid[i][j] = sum / count;
		}
	}
	delete_height_grid(grid_tmp);
}

int create_contiguous_area(unsigned char **grid, unsigned int percent_unwalkable) {
	if (!world->grid || !world->grid->tiles) return 1;
	int i, j, h_min, w_min, h_max, w_max, h, w, count, height;
	h = 0;
	w = 0;
	height = grid[0][0];
	/* mark all as unwalkable and find highest point */
	for (i = 0; i < world->grid->height; i++) {
		for (j = 0; j < world->grid->width; j++) {
			world->grid->tiles[i][j]->walkable = 0;
			if (grid[i][j] > height) {
				height = grid[i][j];
				h = i;
				w = j;
			}
		}
	}
	/* mark highest point as walkable */
	world->grid->tiles[h][w]->walkable = 1;

	height = 0;
	h_min = MAX(0, h - 1);
	w_min = MAX(0, w - 1);
	h_max = MIN(world->grid->height - 1, h + 1);
	w_max = MIN(world->grid->width - 1, w + 1);
	count = world->grid->height * world->grid->width * (100 - percent_unwalkable) / 100;
	while (count > 0) {
		height = 0;
		for (i = h_min; i <= h_max; i++) {
			for (j = w_min; j <= w_max; j++) {
				if (world->grid->tiles[i][j]->walkable) continue;
				if (
					(i > 0 && world->grid->tiles[i - 1][j]->walkable)
					|| (j > 0 && world->grid->tiles[i][j - 1]->walkable)
					|| (i < world->grid->height - 1 && world->grid->tiles[i + 1][j]->walkable)
					|| (j < world->grid->width - 1 && world->grid->tiles[i][j + 1]->walkable)
				) {
					if (grid[i][j] > height) {
						height = grid[i][j];
						h = i;
						w = j;
					}
				}
			}
		}
		world->grid->tiles[h][w]->walkable = 1;
		if (h == h_min) h_min = MAX(0, h_min - 1);
		else if (h == h_max) h_max = MIN(h_max + 1, world->grid->height - 1);
		if (w == w_min) w_min = MAX(0, w_min - 1);
		else if (w == w_max) w_max = MIN(w_max + 1, world->grid->width - 1);
		count--;
	}
	return 0;
}
