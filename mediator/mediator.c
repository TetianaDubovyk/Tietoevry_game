#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <linux/limits.h>
#include "game.h"


const struct unit_type* unit_types[] = {&knight, &swordsman, &archer, &pikeman, &catapult, &ram, &worker, &base}; 

const int max_num_of_units = 1000;
const int max_rounds = 2000;
const int gold_player1 = 2000;
const int gold_player2 = 2000;
const int pl1_base_id = 1;
const int pl2_base_id = 2;


int main(int argc, char *argv[]) { 

    int time_limit = 5; 
    char char_time_limit;

    // Check for provided params
    if (argc > 5 || argc < 4) {
        printf("Usage: map.txt status.txt orders.txt timelimit(optional)"); 
        return -1;
    }
    // Set time_limit
    if (argc == 5) {
        char_time_limit = *argv[4];
    }
    else {
        char_time_limit = '5';
    }

    int current_round = 0;
    char current_player[] = P;

    char map_file[PATH_MAX];
    strcpy (map_file, argv[1]);
    char status_file[PATH_MAX];
    strcpy (status_file, argv[2]);
    char orders_file[PATH_MAX];
    strcpy (orders_file, argv[3]);

    struct map_state* current_map = (struct map_state*) malloc(sizeof(struct map_state));
    if (is_file_empty(map_file)){
        printf("The map file is empty.\n");
        free(current_map);
        return -1;
    }

    // Count the total number of units
    int total_units = get_total_number_of_units(status_file);
    if (total_units < 1){
        total_units = 2;
    }

    struct status_info* current_status = (struct status_info*) calloc(1, sizeof(struct status_info));
    current_status->player1_gold = gold_player1;
    current_status->player2_gold = gold_player2;

    // Allocate memory for current units 
    struct unit* units = (struct unit*) calloc(total_units + max_num_of_units, sizeof(struct unit));

    printf("Welcome to the game!\n");

    while (current_round != max_rounds) {

        current_round++;
        printf("Round # %d\n", current_round);

        // Load map file
        if (!load_map(map_file, current_map)){
            free_memory(current_status, current_map, units);
            return -1;
        }

        if (current_round % 2 == 1){
            strcpy(current_player, P);
        }
        else {
            strcpy(current_player, "Player #2");
        }

        // If the status file is empty, write data about players bases
        if (is_file_empty(status_file)){
            write_first_status_file(current_map, status_file);
            total_units += 2;
        }
        else if (current_round == 1 && !is_file_empty(status_file)){
            if (!validate_status_file(status_file)){
                printf("The status file is invalid.\n");
                free_memory(current_status, current_map, units);
                return -1;
            }
        }

        total_units = get_total_number_of_units(status_file);

        if (sizeof(total_units) > max_num_of_units){
            printf("The number of awailable units are bigger then possible maximum.");
            game_over(max_rounds, max_rounds, current_status, current_map);
            free_memory(current_status, current_map, units);
            return -1;
        }

        // Load status file
        if (!load_status(status_file, current_status, units, current_player)){
            free_memory(current_status, current_map, units);
            return -1;
        }

        // Find bases, update bases data if they are produsing new units 
        if (!find_bases(current_status, &total_units, unit_types, current_player)){
            free_memory(current_status, current_map, units);
            return -1;
        }

        // Run player program with the time limit
        if (!run_player_program(current_status, map_file, status_file, orders_file, char_time_limit, current_player, time_limit)){
            free_memory(current_status, current_map, units);
            return -1;
        }

        // Add gold, if some workers are in mines
        add_gold(current_status);

        // Make orders
        if (!load_orders(orders_file, current_player, current_status, current_map, total_units, unit_types)){
            free_memory(current_status, current_map, units);
            return -1;
        }
        // Check if game is over
        if (game_over(current_round, max_rounds, current_status, current_map)){
            free_memory(current_status, current_map, units);
            return -1;
        }
        // If game is not over write current status into the status file
        else {
            rewrite_status_file(status_file, current_status, total_units, current_player);
        }
    }

    free_memory(current_status, current_map, units);
    return 0;
}

