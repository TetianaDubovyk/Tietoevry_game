#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include "game.h"


const struct unit_type* unit_types[] = {&knight, &swordsman, &archer, &pikeman, &catapult, &ram, &worker, &base}; 

const int max_num_of_units = 1000;
const int max_rounds = 150; // shold be 2000
const int gold_player1 = 2000;
const int gold_player2 = 2000;
const int pl1_base_id = 1;
const int pl2_base_id = 2;

int time_limit = 5; 


// Mediator program should be executed with the following parameters: map.txt status.txt orders.txt timelimit(optional)
int main(int argc, char *argv[]) { 

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

    printf("Welcome to the game!\n");

    char map_file[MAX_STRING_SIZE];
    strcpy (map_file, argv[1]);
    char status_file[MAX_STRING_SIZE];
    strcpy (status_file, argv[2]);
    char orders_file[MAX_STRING_SIZE];
    strcpy (orders_file, argv[3]);

    struct status_info* current_status = (struct status_info*) malloc(sizeof(struct status_info));
    current_status->player1_gold = gold_player1;
    current_status->player2_gold = gold_player2;
    current_status->player1_workers_in_mine = 0;
    current_status->player2_workers_in_mine = 0;

    struct map_state* current_map = (struct map_state*) malloc(sizeof(struct map_state));;

    // Count the total number of units
    int total_units = get_total_number_of_units(status_file);

    // Allocate memory for current units 
    struct unit* units = (struct unit*) malloc(total_units * sizeof(struct unit));

    while (current_round != max_rounds) {

        current_round++;
        printf("Round # %d\n", current_round);

        total_units = get_total_number_of_units(status_file);

        if (sizeof(total_units) > max_num_of_units){
            printf("The number of awailable units are bigger then possible maximum.");
            game_over(max_rounds, max_rounds, current_status->number_of_units_player1, current_status->number_of_units_player2, current_map->player1_base, current_map->player2_base);
            return 0;
        }

        // Load map file
        if (!load_map(map_file, current_map)){
            return 0;
        }

        if (current_round % 2 == 1){
            strcpy(current_player, P);
        }
        else {
            strcpy(current_player, "Player #2");
        }

        // For the first round, if the status file is empty, write data about players bases
        if (current_round == 1 && is_file_empty(status_file)){
            write_first_status_file(current_map, status_file);
            total_units += 2;
        }
        // Load status file
        if (!load_status(status_file, current_status, units, current_player)){
            return 0;
        }

        // Find bases, update bases data if they are produsing new units 
        if (!find_bases(current_status, &total_units, unit_types, current_player)){
            return 0;
        }

        // Run player program with the time limit
        if (!run_player_program(current_status, map_file, status_file, orders_file, char_time_limit, current_player, time_limit)){
            return 0;
        }

        // Add gold, if some workers are in mines
        add_gold(current_status);

        // Make orders
        if (!load_orders(orders_file, current_player, current_status, current_map, total_units, unit_types)){
            return 0;
        }
        // Check if game is over
        else {
            if (game_over(current_round, max_rounds, current_status->number_of_units_player1, current_status->number_of_units_player2, current_map->player1_base, current_map->player2_base)){
                printf("Press Enter to Continue\n");  
                getchar();
                return 0;
            }
            // If game is not over write current status into the status file
            else {
                rewrite_status_file(status_file, current_status, total_units, current_player);
            }
        }
    }

    free(units);
    free(current_status);
    free(current_map); 

    return 0;

}

