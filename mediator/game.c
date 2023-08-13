#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <linux/limits.h>
#include "game.h"


const struct unit_type knight = {'K', 70, 5, 400, 1, 5};
const struct unit_type swordsman = {'S', 60, 2, 250, 1, 3};
const struct unit_type archer = {'A', 40, 2, 250, 5, 3};
const struct unit_type pikeman = {'P', 50, 2, 200, 2, 3};
const struct unit_type ram = {'R', 90, 2, 500, 1, 4};
const struct unit_type catapult = {'C', 50, 2, 800, 7, 6};
const struct unit_type worker = {'W', 20, 2, 100, 1, 2};
const struct unit_type base = {'B', 200, 0};


// Load map file
bool load_map (char file_name[PATH_MAX], struct map_state *map){

    for (int i = 0; i < MAX_SIZE_OF_MAP; i++){
        for (int j = 0; j < MAX_SIZE_OF_MAP; j++){
            map->map[i][j] = UNREACHABLE_DISTANCE;
        }
    }

    for (int i = 0; i < MAX_MINES; i++){
        map->mines[i].x = UNREACHABLE_DISTANCE;
        map->mines[i].y = UNREACHABLE_DISTANCE;
        map->obstacles[i].x = UNREACHABLE_DISTANCE;
        map->obstacles[i].y = UNREACHABLE_DISTANCE;
    }

    // Load map file
    FILE *map_file_ptr;
    map_file_ptr = fopen(file_name, "r");

    int j = 0;
    char *map_line = (char*)calloc(MAX_SIZE_OF_MAP, sizeof(char));
    int mine_id = 0;
    int obstacle_id = 0;
    int base_number = 0;
    map->map_width = 0;
    map->player1_base = '0';
    map->player2_base = '0';
    int read_line_length = 0;


    while(fgets(map_line, MAX_SIZE_OF_MAP, map_file_ptr)) {

        if (map->map_width == 0){
            map->map_width = strlen(map_line) - 2;
        }
        
        read_line_length = strlen(map_line) - 2;

        if (read_line_length < map->map_width || read_line_length > map->map_width){
            printf("The dimensions of the map are uneven.\n");
            fclose(map_file_ptr);
            free(map_line);
            return false;
        }

        for(int i = 0; i <= map->map_width; i++ ){

            map->map[j][i] = map_line[i];

            if (map_line[i] == '1'){ 
                if (map->player1_base == '1' && map->player2_base == '2'){
                    printf("The number of bases on the map is bigger then 2.\n");
                    fclose(map_file_ptr);
                    free(map_line);
                    return false;
                }
                map->player1_base = '1';
                map->bases[base_number].x = j;
                map->bases[base_number].y = i;
                base_number++;
            }
            else if (map_line[i] == '2'){
                if (map->player1_base == '1' && map->player2_base == '2'){
                    printf("The number of bases on the map is bigger then 2.\n");
                    fclose(map_file_ptr);
                    free(map_line);
                    return false;
                }
                map->player2_base = '2';
                map->bases[base_number].x = j;
                map->bases[base_number].y = i;
                base_number++;
            }
            
            else if (map_line[i] == '6'){
                if (mine_id >MAX_MINES){
                    printf("The number of mines on the map is bigger then the default value.\n");
                    fclose(map_file_ptr);
                    free(map_line);
                    return false; 
                }
                map->mines[mine_id].x = j;
                map->mines[mine_id].y = i;
                mine_id++;
            }
            
            else if (map_line[i] == '9'){
                if (obstacle_id >MAX_OBSTACLES){
                    printf("The number of mines on the map is bigger then the default value.\n");
                    fclose(map_file_ptr);
                    free(map_line);
                    return false; 
                }
                map->obstacles[obstacle_id].x = j;
                map->obstacles[obstacle_id].y = i;
                obstacle_id++;
            }
            else if (map_line[i] == '0'){
                continue;
            }
            else {
                printf("The map contains wrong characters.\n");
                fclose(map_file_ptr);
                free(map_line);
                return false;
            }
        }
        j++;
    }

    map->map_height = j;
    map->map_width += 1;

    fclose(map_file_ptr);
    free(map_line);
    return true;
}

bool validate_status_file(char file_name[PATH_MAX]){

    // Load status file
    FILE *status_file_ptr;
    status_file_ptr = fopen(file_name, "r");

    char status_line[PATH_MAX];

    // Patterns for reading
    char curr_money[3] = "%u";
    char status_base[30] = "%c %c %u %u %u %u %c";
    char status_others[30] = "%c %c %u %u %u %u";

    char player, unit_type;
    char unit_type_being_built;
    int unit_id, x, y, strength, curr_m;
    int count_lines = 0;

    while(fgets(status_line, PATH_MAX, status_file_ptr)){

        count_lines++;

        if (sscanf(status_line, status_base, &player, &unit_type, &unit_id, &x, &y, &strength, &unit_type_being_built) == 7){
            if ((unit_type_being_built != '0') || (!validate_player_char(player)) || (!validate_unit_type(unit_type))){
                fclose(status_file_ptr);
                return false;
            }
        }
        else if (sscanf(status_line, status_others, &player, &unit_type, &unit_id, &x, &y, &strength) == 6){
            if ((!validate_player_char(player)) || (!validate_unit_type(unit_type))){
                fclose(status_file_ptr);
                return false;
            }
        }
        else if (sscanf(status_line, curr_money, &curr_m) == 1){
            continue;
        }
    }

    if (count_lines < 2){
        return false;
    }

    fclose(status_file_ptr);
    return true;
}

// Load status file
bool load_status (char file_name[PATH_MAX], struct status_info *status, struct unit* units, char *curr_player){

    // Load status file
    FILE *status_file_ptr;
    status_file_ptr = fopen(file_name, "r");

    char status_line[PATH_MAX];

    // Patterns for reading
    char curr_money[3] = "%u";
    char status_base[30] = "%c %c %u %u %u %u %c";
    char status_others[30] = "%c %c %u %u %u %u";

    char player, unit_type;
    char unit_type_being_built;
    int unit_id, x, y, strength, curr_m;
    int counter_row = 0;

    int count_units_pl1 = 0;
    int count_units_pl2 = 0;
    
    while(fgets(status_line, PATH_MAX, status_file_ptr)){

        // Money line
        if (sscanf(status_line, curr_money, &curr_m) == 1){
            // Save the amount of gold for the current player
            if (strcmp(curr_player, P) == 0){
                status->player1_gold = curr_m;
            }
            else {
                status->player2_gold = curr_m;
            }
        }
        else {
            // Status of the bases
            if (sscanf(status_line, status_base, &player, &unit_type, &unit_id, &x, &y, &strength, &unit_type_being_built) == 7){

                if (!validate_status_base_line_chars(player, unit_type, unit_type_being_built)){
                    fclose(status_file_ptr);
                    return false;
                }

                struct coordinates unit_coord;
                unit_coord.x = x;
                unit_coord.y = y;
                
                units[counter_row].player = player;
                units[counter_row].unit_type = &base;
                units[counter_row].unit_type_being_built = unit_type_being_built;
                if (units[counter_row].unit_type_being_built == '0'){
                    units[counter_row].time_to_build = 0;
                }
                else {
                    units[counter_row].time_to_build = status->units[counter_row].time_to_build;
                }
                units[counter_row].unit_id = unit_id;
                units[counter_row].coordinates = unit_coord;
                units[counter_row].current_strenght = strength;
                units[counter_row].remaining_moves = 0;
                counter_row++;
            }
            // Status of other units
            else if (sscanf(status_line, status_others, &player, &unit_type, &unit_id, &x, &y, &strength) == 6){     

                if (!validate_player_char(player) || !validate_unit_type(unit_type)){
                    fclose(status_file_ptr);
                    return false;
                }

                struct coordinates unit_coord;
                unit_coord.x = x;
                unit_coord.y = y;
                
                units[counter_row].player = player;

                if (unit_type == 'K'){
                    units[counter_row].unit_type = &knight;
                }
                else if (unit_type == 'S'){
                    units[counter_row].unit_type = &swordsman;
                }
                else if (unit_type == 'A'){
                    units[counter_row].unit_type = &archer;
                }
                else if (unit_type == 'P'){
                    units[counter_row].unit_type = &pikeman;
                }
                else if (unit_type == 'R'){
                    units[counter_row].unit_type = &ram;
                }
                else if (unit_type == 'C'){
                    units[counter_row].unit_type = &catapult;
                }
                else if (unit_type == 'W'){
                    units[counter_row].unit_type = &worker;
                }

                units[counter_row].unit_type_being_built = '0';
                units[counter_row].time_to_build = 0;
                units[counter_row].unit_id = unit_id;
                units[counter_row].coordinates = unit_coord;
                units[counter_row].current_strenght = strength;
                units[counter_row].remaining_moves = units[counter_row].unit_type->speed;

                counter_row++;
            }
            else {
                printf("The status file does not meet the requirements criteria.\n");
                fclose(status_file_ptr);
                return false;
            }
            
            // Count the number of units of each player
            if (player == 'P'){
                count_units_pl1++;
            }
            else if (player == 'E'){
                count_units_pl2++;
            }
        }
    }

    status->units = units;
    status->number_of_units_player1 = count_units_pl1;
    status->number_of_units_player2 = count_units_pl2;

    fclose(status_file_ptr);
    return true;
}

// Check if file is empty
bool is_file_empty(char file_name[PATH_MAX]){

    // Open a file
    FILE *fp;
    fp = fopen(file_name, "r");

    if (fp == NULL) {
        printf("Cannot open a file: %s.\n", file_name);
        fclose(fp);
        return false;
    }

    fseek (fp, 0, SEEK_END);
    int size = ftell(fp);
    fclose(fp);

    return size == 0;
}

bool validate_status_base_line_chars(char player, char unit_type, char unit_type_being_built){

    if (!validate_player_char(player)){
        printf("The status file contains invalid character of a player.\n");
        return false;
    }

    if (unit_type_being_built == '0'){
        if (!validate_unit_type(unit_type)){
            return false;
        }
    }
    else {
        if (!validate_unit_type(unit_type) || !validate_unit_type(unit_type_being_built)){
            printf("The status file contains invalid unit types.\n");
            return false;
        }
    }
    
    return true;
}

bool validate_player_char(char p){

    char players[] = "PE";

    if ((p != players[0] && p != players[1]) || (isalpha(p) == 0)){
        return false;
    }

    return true;
}

bool validate_unit_type(char type){

    if (isalpha(type) != 0){

        char char_types[] = "KSAPCRWB";

        for (int i = 0; i < strlen(char_types); i++){
            if (type == char_types[i]){
                return true;
            }
        }
    }

    return false;
}

bool write_first_status_file(struct map_state *map_ptr, char file_name[PATH_MAX]){

    FILE *status_f;
    status_f = fopen(file_name, "w" );

    char default_gold[] = "2000";
    char new_line[] = "\n";

    fwrite(default_gold, 1, strlen(default_gold),status_f );
    fwrite(new_line, 1, strlen(new_line), status_f);

    char pattern[] = "%c %c %c %s %s %s %c";

    char *base_x = (char*)malloc(2 * sizeof(int));
    char *base_y = (char*)malloc(2 * sizeof(int));
    char *base_res = (char*)malloc(2 * sizeof(pattern));

    char player1 = 'P';
    char player2 = 'E';
    char base_notation = 'B';
    char health[] = "100";
    char type_to_buid = '0';
    char unit_id = '1';

    for (int index = 0; index <=1; index++){
        sprintf(base_x, "%d", map_ptr->bases[index].x);
        sprintf(base_y, "%d", map_ptr->bases[index].y);

        if (index == 0){
            sprintf(base_res, pattern, player1, base_notation, unit_id, base_x, base_y, health, type_to_buid);
        }
        else if (index == 1){
            sprintf(base_res, pattern, player2, base_notation, unit_id, base_x, base_y, health, type_to_buid);
        }
        
        fwrite(base_res, 1, strlen(base_res), status_f);
        fwrite(new_line, 1, strlen(new_line), status_f);
        unit_id = '2';
    }

    fclose(status_f);

    free(base_x);
    free(base_y);
    free(base_res);
    return true;
}

// Read the orders file, change the status and the map if needed
bool load_orders (char file_name[PATH_MAX], char *player, struct status_info *status, struct map_state *map, int num_of_units, const struct unit_type* unittypes[]){

    // Open orders file
    FILE *orders_file_ptr;
    orders_file_ptr = fopen(file_name, "r");

    char orders_line[30];

    // Patterns for reading
    char move_unit[20] = "%u M %u %u";
    char attack_unit[20] = "%u A %u";
    char build_unit[20] = "%u B %c";

    int id1 = 0; int id2 = 0; 
    int x = 0;
    int y = 0;
    char unit_to_build;

    // Read orders file
    while(fgets(orders_line, 30, orders_file_ptr)){

        // Move
        if (sscanf(orders_line, move_unit, &id1, &x, &y) == 3){
            printf("Unit of id %u Moves to (%u:%u)\n", id1, x, y);
            if (!move(status, id1, x, y, num_of_units, player, map)){
                fclose(orders_file_ptr);
                return false;
            }
        }
        // Attack
        if (sscanf(orders_line, attack_unit, &id1, &id2) == 2){
            printf("Unit of id %u Attack the one with id %u\n", id1, id2);
            if (!attack(status, id1, id2, num_of_units, player, map, unittypes)){
                fclose(orders_file_ptr);
                return false;
            }
        }
        // Build
        if (sscanf(orders_line, build_unit, &id1, &unit_to_build) == 2){
            printf("Build \'%c\' unit in the base of id %u\n", unit_to_build, id1);
            if (!build(status, id1, unit_to_build, num_of_units, player, unittypes)){
                fclose(orders_file_ptr);
                return false;
            }
        }
    }

    fclose(orders_file_ptr);
    return true;
}

// Functon to count the distance of a unit for speed and attack range
int get_distance(int x1, int x2, int y1, int y2) {
    int result = abs(x1 - x2) + abs(y1 - y2);
    return result;
}

// Function to move a unit
bool move(struct status_info *status, int id1, int x, int y, int num_of_units, char *player, struct map_state *map){

    for(int i = 0; i < num_of_units; i++){
        // Find unit to move
        if (status->units[i].unit_id == id1){ 

            int x1 = status->units[i].coordinates.x;
            int y1 = status->units[i].coordinates.y;

            // Check if unit is a base 
            if (status->units[i].unit_type->notation == 'B'){ 
                printf("The type of a unit: %c cannot move, only build. '%s' has been disqualified!\n", status->units[i].unit_type->notation, player);
                print_winner(player);
                return false;
            }

            // Validate the given coordinates 
            if (!check_map_range(x, y, map)){
                printf("The coordinates is out of the map range. The action is impossible. '%s' has been disqualified!\n", player);
                print_winner(player);
                return false;
            }

            if (is_obstacle(x, y, map)){
                printf("There is an obstacle on the coordinates. The action is impossible. '%s' has been disqualified!\n", player);
                print_winner(player);
                return false;
            }

            if (is_base(x, y, map)){
                printf("There is a base on the coordinates. The action is impossible. '%s' has been disqualified!\n", player);
                print_winner(player);
                return false;
            }
            
            // Check if the coordinates for the move is taken by an opponents unit
            if (is_coordinates_taken(status, x, y, num_of_units, status->units[i])){
                printf("The coordinates is taken by an opponents unit! '%s' has been disqualified!\n", player);
                print_winner(player);
                return false;
            }

            int distance = get_distance(x1, x, y1, y);
            if (distance > status->units[i].remaining_moves){
                printf("The move is impossible. The given coordinates is out of the units velocity.\n");
                print_winner(player);
                return false;
            }
            // If unit is worker and the coordinates is a mine
            if (status->units[i].unit_type->notation == 'W' && is_mine(x, y, map)){
                if (strcmp(player, P) == 0){
                    status->player1_workers_in_mine ++;
                }
                else {
                    status->player2_workers_in_mine ++;
                }
            }
            // If unit is not a worker
            else if (status->units[i].unit_type->notation != 'W' && is_mine(x, y, map)){
                printf("There is a mine on the coordinates. The action is impossible. '%s' has been disqualified!\n", player);
                print_winner(player);
                return false;
            }
            // If the move is possible, move to the given coord
            status->units[i].coordinates.x = x;
            status->units[i].coordinates.y = y;
            status->units[i].remaining_moves = status->units[i].remaining_moves - distance;
            break;         
        }
    
        // If id of unit cannot be found
        if (i == (num_of_units - 1)){
            printf("Unit of id:%d cannot be found! '%s' has been disqualified!\n", id1, player);
            print_winner(player);
            return false;
        }
    }
    return true;
}

// Function to start buiding a new unit on the base
bool build(struct status_info *status, int id, char unit_to_build, int num_of_units, char *player, const struct unit_type* unittypes[]){

    for(int i = 0; i < num_of_units; i++){

        if (status->units[i].unit_id == id){

            if(status->units[i].unit_type->notation != 'B'){
                printf("The given id is not the id of a base. %s has been disqualified!\n", player);
                print_winner(player);
                return false;
            }

            // Check if base with the given id is building a new unit
            if (status->units[i].unit_type_being_built != '0'){
                printf("The base with the given id is alredy building a unit! %s has been disqualified!\n", player);
                print_winner(player);
                return false;
            }
            else {
                // Start building
                status->units[i].unit_type_being_built = unit_to_build;

                // Find a type of a unit to build
                const struct unit_type *type = get_type_by_notation(unit_to_build, unittypes);

                // Update the base data
                int time = type->time_to_build;
                status->units[i].time_to_build = time;

                if (strcmp(player, P) == 0){
                    if (status->player1_gold < type->cost){
                        printf("Not enough gold to buid a new unit! %s has been disqualified!\n", P);
                        printf("Player #2 wins!\n");
                        return false;
                    }
                    else {
                        status->player1_gold -= type->cost;
                    }
                }
                else{
                    if (status->player2_gold < type->cost){
                        printf("Not enough gold to buid a new unit! Player #2 has been disqualified!\n");
                        printf("Player #1 wins!\n");
                        return false;
                    }
                    else {
                        status->player2_gold -= type->cost;
                    }
                }
                break;
            }
        }
    }
    return true;
}

// Function to attack a unit 
bool attack(struct status_info *status, int id1, int id2, int num_of_units, char *player, struct map_state *map, const struct unit_type* unittypes[]){

    int attacker_status_number = 0;
    int target_status_number = 0;

    struct unit *target = (struct unit*) malloc(sizeof(struct unit));
    struct unit *attacker = (struct unit*) malloc(sizeof(struct unit));

    // Find attacker and target units
    for(int i = 0; i < num_of_units; i++){
        // Target unit 
        if (status->units[i].unit_id == id2){
            *target = status->units[i];
            target_status_number = i;

            // Attaker unit 
            for (int j = 0; j < num_of_units; j++){
                if (status->units[j].unit_id == id1){
                    *attacker = status->units[j];
                    attacker_status_number = j;

                    // Check if attacker unit is a base 
                    if (attacker->unit_type->notation == 'B'){ 
                        printf("The type of a unit: %c cannot attack, only build. '%s' has been disqualified!\n", attacker->unit_type->notation, player);
                        print_winner(player);
                        free(target);
                        free(attacker);
                        return false;
                    }

                    // If attack is on the own unit
                    if (attacker->unit_id == target->unit_id || attacker->player == target->player){
                        printf("Attacking our own unit! '%s' has been disqualified!\n", player);
                        print_winner(player);
                        free(target);
                        free(attacker);
                        return false;
                    }
                    
                    break;
                }

                // If attacker id cannot be found
                if (j == (num_of_units - 1)){
                    printf("Attacker unit of id:%d cannot be found. '%s' has been disqualified!\n", id1, player);
                    print_winner(player);
                    free(target);
                    free(attacker);
                    return false;
                }
            }
            break;
        }
        // If target id cannot be found
        if (i == (num_of_units - 1)){
            printf("Target unit of id:%d cannot be found. '%s' has been disqualified!\n", id2, player);
            print_winner(player);
            free(target);
            free(attacker);
            return false;
        }
    }

    // Calculate the range for attack
    int reach_distance = get_distance(attacker->coordinates.x, target->coordinates.x, attacker->coordinates.y, target->coordinates.y);

    // Check if target is in the attacker range
    if (reach_distance > attacker->unit_type->range_of_attack){
        printf("The attack is impossible. The given coordinates is out of the units reach.\n");
        print_winner(player);
        free(target);
        free(attacker);
        return false;
    }
    // If attack is posiible
    else {

        int demage = get_attack_damage(attacker, target, unittypes);

        if (demage >= 0){
            target->current_strenght -= demage;
            attacker->remaining_moves--;

            // Update the target data
            status->units[target_status_number].current_strenght = target->current_strenght;
            status->units[attacker_status_number].remaining_moves = attacker->remaining_moves;

            // If target is defeated
            if (target->current_strenght <= 0){
                // If target is a base
                if (target->unit_type->notation == 'B'){
                    if (target->unit_id == 1){
                    map->player1_base = '0';
                    }
                    else {
                        map->player2_base = '0';
                    }
                }
                // If target is not a base
                else {
                    if (strcmp(player, P) == 0){
                        status->number_of_units_player1--;
                    }
                    else {
                        status->number_of_units_player1--;
                    }
                }
            }

            free(target);
            free(attacker);
            return true;
        }
    }

    free(target);
    free(attacker);
    return false;
}

// Check if the coordinates for the move is taken by opponents unit
bool is_coordinates_taken(struct status_info *status, int x, int y, int num_of_units, struct unit u){

    for(int i = 0; i < num_of_units; i++){
        if (status->units[i].coordinates.x == x && status->units[i].coordinates.y == y && status->units[i].player != u.player){
            return true;
        }
    }

    return false;
}

// Check if the coordinates for a move is out of map range
bool check_map_range(int x, int y, struct map_state *map_ptr){
    return (x >= 0 && x < map_ptr->map_height && y >= 0 && y < map_ptr->map_width + 1); // changed to < map_width, from <=
}

// Check if a mine on the coordinates
bool is_mine(int x, int y, struct map_state *map_ptr){
    for(int m = 0; m < MAX_MINES; m++){
        if (map_ptr->mines[m].x == x && map_ptr->mines[m].y == y){
            return true;   
        }
    }
    return false;
}

// Check if a base on the coordinates
bool is_base(int x, int y, struct map_state *map_ptr){
    for(int b = 0; b < 2; b++){
        if (map_ptr->bases[b].x == x && map_ptr->bases[b].y == y){
            return true;
        }
    }
    return false;
}

// Check if an obstacle on the coordinates
bool is_obstacle(int x, int y, struct map_state *map_ptr){

    int i = 0;
    while (map_ptr->obstacles[i].x != -1 && map_ptr->obstacles[i].y != -1){
        if (map_ptr->obstacles[i].x == x && map_ptr->obstacles[i].y == y){
            return true;
        }
        i++;
    }
    return false;
}

// Find bases to update their data if they are building new units + create new units
bool find_bases(struct status_info *status, int *total_num_of_units, const struct unit_type* unittypes[], char player[]){

    int id1 = 1; // player1 base
    int id2 = 2; // player2 base
    
    // There are only 2 bases and they are the first two units in status.units
    for(int i = 0; i < 2; i++){ 

        // For player #1
        if (status->units[i].unit_id == id1){
            if (is_base_busy(status, i, id1, *total_num_of_units, unittypes, player)){
                // Update the total number of units
                *total_num_of_units = status->number_of_units_player1 + status->number_of_units_player2;
            }
            continue;
        }
        // For player #2
        if (status->units[i].unit_id == id2){
            if (is_base_busy(status, i, id2, *total_num_of_units, unittypes, player)){
                *total_num_of_units = status->number_of_units_player1 + status->number_of_units_player2;
            }
            continue;
        }
        // If id of unit cannot be found
        if (i == 2 ){
            printf("Base of unit:%c cannot be found!\n", status->units[i].player);
            return false;
        }
    }

    return true;
}

// Check if base is done producing a new unit, update units
bool is_base_busy(struct status_info *status, int unit, int id, int total_num_of_units, const struct unit_type* unittypes[], char player[]){

    // Update base if it is produces new units
    if (status->units[unit].unit_id == id && status->units[unit].time_to_build >= 1){
        status->units[unit].time_to_build--;

        // If the base is finished to produce a new unit
        if (status->units[unit].unit_id == id && status->units[unit].time_to_build == 0 && status->units[unit].unit_type_being_built != '0'){
            
            int ids_counter = 0;
            int id_tepm = 0;
            int next_unit = total_num_of_units;

            if (id == 1){
                for (int i = 0; i < total_num_of_units; i++){
                    if (status->units[i].unit_id % 2 == 1){
                        id_tepm = status->units[i].unit_id;
                        if (ids_counter < id_tepm){
                            ids_counter = id_tepm;
                        }
                    }
                }
                status->number_of_units_player1++;
            }
            else {
                for (int i = 0; i < total_num_of_units; i++){
                    if (status->units[i].unit_id % 2 == 0){
                        id_tepm = status->units[i].unit_id;
                        if (ids_counter < id_tepm){
                            ids_counter = id_tepm;
                        }
                    }
                }
                status->number_of_units_player2++;
            }

            ids_counter += 2;

            // Create a new unit
            status->units[next_unit].coordinates = status->units[unit].coordinates;
            status->units[next_unit].player = status->units[unit].player;
            status->units[next_unit].unit_type = get_type_by_notation(status->units[unit].unit_type_being_built, unittypes);
            status->units[next_unit].current_strenght = status->units[next_unit].unit_type->strength;
            status->units[next_unit].time_to_build = 0;
            status->units[next_unit].unit_type_being_built = 0;
            status->units[next_unit].unit_id = ids_counter;
            
            // Set the base field unit_type_being_built to '0'
            status->units[unit].unit_type_being_built = '0';
        }
        return true;
    }

    return false;
}

// Count the total number of units
int get_total_number_of_units(char file_name[PATH_MAX]){

    int units_counter = -1;

    // Load status file
    FILE *status_file;

    status_file = fopen(file_name, "r");
    char status_line[PATH_MAX];

    if (status_file == NULL){
        return 0;
    }

    while(fgets(status_line, PATH_MAX, status_file)){
        units_counter++;
    }

    fclose(status_file);
    return units_counter; 
}

// Find the demage of an attack
int get_attack_damage(struct unit *attacker, struct unit *target, const struct unit_type* unittypes[]){

    // Table of attack
    const int attack_table[NUM_OF_TYPES][NUM_OF_TYPES] =
    {
        {35, 35, 35, 35, 35, 50, 35, 35},
        {30, 30, 30, 20, 20, 30, 30, 30},
        {15, 15, 15, 15, 10, 10, 15, 15},
        {35, 15, 15, 15, 15, 10, 15, 10},
        {40, 40, 40, 40, 40, 40, 40, 50},
        {10, 10, 10, 10, 10, 10, 10, 50},
        {5, 5, 5, 5, 5, 5, 5, 1},
        {0, 0, 0, 0, 0, 0, 0, 0}
    };

    int attacker_index = get_unit_attack_index_by_notation(attacker->unit_type->notation, unittypes);
    int target_index = get_unit_attack_index_by_notation(target->unit_type->notation, unittypes);

    if ((attacker_index < 0 || attacker_index > NUM_OF_TYPES) || (target_index < 0 || target_index > NUM_OF_TYPES)){
        printf("Units are out of range (attack table). Exiting the program....");
        return -1;
    }    

    return attack_table[attacker_index][target_index];
}

// Find a unit index in the units types
int get_unit_attack_index_by_notation(char unit_notation, const struct unit_type* unittypes[]){

    for (int i = 0; i < NUM_OF_TYPES; i++){
        if (unittypes[i]->notation == unit_notation){
            return i;
        }
    }
    return -1;
}

const struct unit_type* get_type_by_notation(char unit_notation, const struct unit_type* unittypes[]){
    for (int i = 0; i < NUM_OF_TYPES; i++){
        if (unittypes[i]->notation == unit_notation){
            return unittypes[i];
        }
    }
    return unittypes[0];
}

void print_winner(char *player){
    if (strcmp(player, P)){
        printf("Player #2 wins!");
    }
    else {
        printf("Player #1 wins!");
    }
}

char swap_players(char *current_p, char units_p){

    char p2[] = "Player #2";

    if ((strcmp(current_p, P) == 0) && units_p == 'P'){
        units_p = 'E';
        return units_p;
    }
    if ((strcmp(current_p, P) == 0) && units_p == 'E'){
        units_p = 'P';
        return units_p;
    }
    if ((strcmp(current_p,p2) == 0) && units_p == 'E'){
        units_p = 'P';
        return units_p;
    }
    if ((strcmp(current_p,p2) == 0) && units_p == 'P'){
        units_p = 'E';
        return units_p;
    }

    return units_p;
}

// Add gold, if some workers are in mines
void add_gold(struct status_info *status){

    if (status->player1_workers_in_mine >= 1){
        status->player1_gold = status->player1_gold + status->player1_workers_in_mine * 50;
    }
    if (status->player2_workers_in_mine >= 1){
        status->player2_gold = status->player2_gold + status->player2_workers_in_mine * 50;
    }
}

// Write current status of units into a status file
void rewrite_status_file(char file_name[PATH_MAX], struct status_info *status, int num_of_units, char player[]){

    char *gold = (char*)malloc(2 * sizeof(int));
    if (strcmp(player, P) == 0){
        sprintf(gold, "%d", status->player2_gold);
    }
    else {
        sprintf(gold, "%d", status->player1_gold);
    }
    char pl = 'P';
    char u_notation = 'K';
    char u_type_built = 'K';

    char *unitid = (char*)malloc(2 * sizeof(int));
    char *ch_x = (char*)malloc(2 * sizeof(int));
    char *ch_y = (char*)malloc(2 * sizeof(int));
    char *h = (char*)malloc(2 * sizeof(int));
    char *base_result = (char*)malloc(7 * sizeof(int));
    char *result = (char*)malloc(6 * sizeof(int));
    char new_line[] = "\n";

    FILE *st_file;
    st_file = fopen(file_name, "w" );

    // Write first line 
    fwrite(gold, 1, strlen(gold),st_file );
    fwrite(new_line, 1, strlen(new_line), st_file);

    // Read all data about a unit from current status
    for(int i = 0; i < num_of_units; i++){

        // If unit is not defeated
        if (status->units[i].current_strenght > 0){

            pl = status->units[i].player;

            // Swap players notations for next player to read 
            pl = swap_players(player, pl);

            u_notation = status->units[i].unit_type->notation;
            sprintf(unitid, "%d", status->units[i].unit_id); 
            sprintf(ch_x, "%d", status->units[i].coordinates.x);
            sprintf(ch_y, "%d", status->units[i].coordinates.y);
            sprintf(h, "%d", status->units[i].current_strenght);

            // Base unit
            if (u_notation == 'B'){
                u_type_built = status->units[i].unit_type_being_built;
                sprintf(base_result, "%c %c %s %s %s %s %c", pl, u_notation, unitid, ch_x, ch_y, h, u_type_built);
                fwrite(base_result, 1, strlen(base_result), st_file);
                fwrite(new_line, 1, strlen(new_line), st_file);
                continue;
            }
            // Other units
            sprintf(result, "%c %c %s %s %s %s", pl, u_notation, unitid, ch_x, ch_y, h);
            fwrite(result, 1, strlen(result), st_file);
            fwrite(new_line, 1, strlen(new_line), st_file);
            continue;
        }
        // If unit is defeated
        else {
            continue;
        }
    }

    free(gold);
    free(unitid);
    free(ch_x);
    free(ch_y);
    free(h);
    free(base_result);
    free(result);

    fclose(st_file);
}

// Run players program 
bool run_player_program(struct status_info *status, char *map_file, char *status_file, char *orders_file, char t_limit, char *player, char limit){

    int int_time_limit = 0;

    // If were given particular time limit
    if (t_limit > 0){
        int_time_limit = t_limit - '0';
    }
    // Set time limit to default value
    else {
        int_time_limit = limit;
    }
    
    char player_program[100] = "/home/tati/Downloads/Tietoevry_game-main/mediator/player.exe";
    char player_argv[PATH_MAX];

    char *time_lim = (char*)malloc(sizeof(time_lim));
    sprintf(time_lim, "%d", int_time_limit);
    sprintf(player_argv, "%s %s %s %s %s", player_program, map_file, status_file, orders_file, time_lim);

    // Set timer
    int result = 0;
    int start = 0;
    int end = 0;

    time_t t = time(NULL);
    if(t != (time_t)(-1)){
        start = gmtime(&t)->tm_sec;
    }
    
    // Run player program
    int status_code = 0;
    status_code = system(player_argv);

    t = time(NULL);
    if(t != (time_t)(-1)){
        end = gmtime(&t)->tm_sec;
    }

    if (status_code == -1){
        printf("Something went wrong. Player couldn't make his move.");
        free(time_lim);
        return false;
    }
    else {

        result += (end-start);

        if (result > int_time_limit){
            printf("Time limit exceeded. %s is disqualified.\n", player);
            free(time_lim);
            return false;
        }
    }
    
    free(time_lim);
    return true;
}

// Get the winner, based on player bases and moves
bool game_over (int round, int max_rounds, struct status_info *status, struct map_state *map){
    
    // If one of the bases is missing
    if (map->player1_base == '1' && map->player2_base == '0'){
        printf("The enemy base is defeated. Game over! Player #1 wins!\n");
        return true;
    }
    else if (map->player2_base == '2' && map->player1_base == '0'){
        printf("The enemy base is defeated. Game over! Player #2 wins!\n");
        return true;
    }

    // If the maximum number of moves reached
    if (round >= max_rounds){
        printf("Game is over, the maximum number of moves are reached. \n");

        if (status->number_of_units_player1 > status->number_of_units_player2){
            printf("The winner is Player #1, with maximum number of units:%d.\n", status->number_of_units_player1);
            return true;
        }
        if (status->number_of_units_player1 < status->number_of_units_player2){
            printf("The winner is Player #2, with maximum number of units:%d.\n", status->number_of_units_player2);
            return true;
        }
        else {
            printf("It is a tie!\n");
            return true;
        }
    }

    return false;
}

void free_memory (struct status_info *status, struct map_state *map, struct unit *unit){

    free(status);
    free(map);
    free(unit);

}
