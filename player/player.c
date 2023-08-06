#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "queue.h"


const struct unit_type* unit_types[] = {&knight, &swordsman, &archer, &pikeman, &catapult, &ram, &worker, &base}; 


int main(int argc, char *argv[]) { 

    char char_time_limit;

    // Check for provided params
    if (argc > 5 || argc < 4) {
        printf("Usage: map.txt status.txt orders.txt timelimit(optional)"); 
        return -1;
    }
    // Set the time limit to the given value
    if (argc == 5) {
        char_time_limit = *argv[4];
    }
    else {
        // Set the time limit to a default value
        char_time_limit = '5'; 
    }

    char map_file_name[MAX_STRING_SIZE];
    strcpy (map_file_name, argv[1]);
    char status_file_name[MAX_STRING_SIZE];
    strcpy (status_file_name, argv[2]);
    char orders_file_name[MAX_STRING_SIZE];
    strcpy (orders_file_name, argv[3]);

    // Load map file
    struct map_state* current_map = (struct map_state*) malloc(sizeof(struct map_state));
    if (!load_map(map_file_name, current_map)){
        return -1;
    }

    // Count the total number of units
    int total_units = get_total_number_of_units(status_file_name);

    // Allocate memory for units
    struct unit* units = (struct unit*) malloc(total_units * sizeof(struct unit));

    // Load status file
    struct status_info* current_status = (struct status_info*) malloc(sizeof(struct status_info));
    if (!load_status(status_file_name, current_status, units)){
        return -1;
    }

    // Find all data about mines
    struct check_mine_result* mines_result = (struct check_mine_result*) malloc(sizeof(struct check_mine_result));
    check_mines(current_map, current_status, mines_result, total_units);

    // Update the amount of gold, based on the number of workers in mines
    update_gold(current_status, mines_result);

    to_empty_orders_file(orders_file_name);
    
    // Fill the matrix of distances
    int *dist_beetween_cells = calculate_distances(current_map, current_map->map_height, current_map->map_width, current_status, total_units);

    // Claculate vulnerability factors for all units
    vulnerability_factor(current_status, current_map, unit_types, current_map->map_height, current_map->map_width, dist_beetween_cells);

    struct unit* our_base = (struct unit*) malloc(sizeof(struct unit));
    struct unit* enemy_base = (struct unit*) malloc(sizeof(struct unit));

    // Find our base and the enemys base
    find_bases(current_status, our_base, enemy_base);

    /*
    Each turn contains a set of tasks in the following order:
	- send workers to mines
	- protect our base if it's in danger, we define the relative forces balance near our base and summon all appropriate units to the base and attack potential threats
	- attack enemy base, we define our free units and send them to attack enemy base or its protectors
	- build new unit if possible
    */
    
    send_workers_to_mines(mines_result, current_map, current_status, dist_beetween_cells, mines_result, total_units, unit_types, orders_file_name);

    protect_our_base(current_map, current_status, our_base, dist_beetween_cells, mines_result, total_units, unit_types, orders_file_name);

    attack_enemy_base(current_map, current_status, enemy_base, dist_beetween_cells, mines_result, total_units, unit_types, orders_file_name);

    build_new_unit(current_status, our_base, unit_types, orders_file_name);

    free(dist_beetween_cells);
    free(our_base);
    free(enemy_base);
    free(current_status);
    free(current_map);
    free(mines_result);

    return 0;
}

