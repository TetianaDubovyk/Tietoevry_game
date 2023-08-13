#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <linux/limits.h>
#include "player_game.h"
#include "queue.h"


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
    int count_mines = 0;
    int count_obstacles = 0;
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
                if (count_mines >MAX_MINES){
                    printf("The number of mines on the map is bigger then default value.\n");
                    fclose(map_file_ptr);
                    free(map_line);
                    return false; 
                }
                map->mines[mine_id].x = j;
                map->mines[mine_id].y = i;
                mine_id++;
                count_mines++;
            }
            
            else if (map_line[i] == '9'){
                if (count_obstacles >MAX_OBSTACLES){
                    printf("The number of mines on the map is bigger then default value.\n");
                    fclose(map_file_ptr);
                    free(map_line);
                    return false; 
                }
                map->obstacles[obstacle_id].x = j;
                map->obstacles[obstacle_id].y = i;
                obstacle_id++;
                count_obstacles++;
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
    map->num_of_mines = count_mines;
    map->num_of_obstacles = count_obstacles;
    map->map_width += 1;

    fclose(map_file_ptr);
    free(map_line);
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

// Load status file
bool load_status (char file_name[PATH_MAX], struct status_info *status, struct unit* units){

    // Load status file
    FILE *status_file_ptr;
    status_file_ptr = fopen(file_name, "r");

    if(status_file_ptr == NULL) {
        printf("Cannot open a file: %s.\n", file_name);
        fclose(status_file_ptr);
        return false;
    }

    char status_line[PATH_MAX];

    // Patterns for reading
    char curr_money[] = "%u";
    char status_base[] = "%c %c %u %u %u %u %c";
    char status_others[] = "%c %c %u %u %u %u";

    char player = '0';
    char unit_type = '0';
    char unit_type_being_built = '0';
    int unit_id = 0; int x = 0; int y = 0;
    int strength = 0; int curr_m = 0;
    int counter_row = 0;

    int count_units = 0;
    
    while(fgets(status_line, PATH_MAX, status_file_ptr)){

        // Money line
        if (sscanf(status_line, curr_money, &curr_m) == 1){
            // Save the amount of gold for the current player
            status->gold = curr_m;
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
                units[counter_row].time_to_build = 0;
                units[counter_row].unit_id = unit_id;
                units[counter_row].coordinates = unit_coord;
                units[counter_row].current_strenght = strength;
                units[counter_row].vulnerability_factor = 0.0;
                units[counter_row].weightened_strenght = 0.0;
                units[counter_row].busy = false;
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
                if (unit_type == 'S'){
                    units[counter_row].unit_type = &swordsman;
                }
                if (unit_type == 'A'){
                    units[counter_row].unit_type = &archer;
                }
                if (unit_type == 'P'){
                    units[counter_row].unit_type = &pikeman;
                }
                if (unit_type == 'R'){
                    units[counter_row].unit_type = &ram;
                }
                if (unit_type == 'C'){
                    units[counter_row].unit_type = &catapult;
                }
                if (unit_type == 'W'){
                    units[counter_row].unit_type = &worker;
                }

                units[counter_row].unit_type_being_built = '0';
                units[counter_row].time_to_build = 0;
                units[counter_row].vulnerability_factor = 0.0;
                units[counter_row].unit_id = unit_id;
                units[counter_row].busy = false;
                units[counter_row].remaining_moves = units[counter_row].unit_type->speed;
                units[counter_row].coordinates = unit_coord;
                units[counter_row].current_strenght = strength;
                units[counter_row].weightened_strenght = 0.0;

                counter_row++;
            }
            else {
                printf("The status file does not meet the requirements criteria.\n");
                fclose(status_file_ptr);
                return false;
            }
            // Count the number of units
            count_units++;
        }
    }

    status->units = units;
    status->number_of_units = count_units;
    status->is_base_in_danger = false;

    fclose(status_file_ptr);
    return true;
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

// Find data about all mines
void check_mines(struct map_state *map, struct status_info *status, struct check_mine_result *result, int num_of_units){

    for (int i = 0; i < MAX_MINES; i++){
        result->enemy_mines[i].x = UNREACHABLE_DISTANCE;
        result->enemy_mines[i].y = UNREACHABLE_DISTANCE;
        result->free_mines[i].x = UNREACHABLE_DISTANCE;
        result->free_mines[i].y = UNREACHABLE_DISTANCE;
        result->our_mines[i].x = UNREACHABLE_DISTANCE;
        result->our_mines[i].y = UNREACHABLE_DISTANCE;
    }

    int counter_enemies = 0;
    int counter_ours = 0;
    int counter_free = 0;
    int count_workers = 0;

    for (int i = 0; i < map->num_of_mines; i++){

        bool free_mine = true;

        for (int j = 0; j < num_of_units; j++){
            // If mine is occupied
            if (map->mines[i].x == status->units[j].coordinates.x && map->mines[i].y == status->units[j].coordinates.y){
                
                free_mine = false;

                // If a mine is occupied by an enemies unit
                if (status->units[j].player == 'E'){
                    result->enemy_mines[counter_enemies] = map->mines[i];
                    counter_enemies++;
                    break;
                }
                else {
                    result->our_mines[counter_ours] = map->mines[i];
                    count_workers++;
                    counter_ours++;
                    break;
                }
            }
        }
        if (free_mine){
            result->free_mines[counter_free] = map->mines[i];
            counter_free++;
        }
    }

    result->num_of_enemy_mines = counter_enemies;
    result->num_of_our_mines = counter_ours;
    result->num_of_free_mines = counter_free;
    result->num_of_mining_workers = count_workers;

}

void update_gold(struct status_info *status, struct check_mine_result *result){

    status->gold = status->gold + result->num_of_mining_workers * 50;

}

void find_bases(struct status_info *status, struct unit* our_base, struct unit* enemy_base){
    // Concidering that the 2nd and 3rd lines in status.txt is always a data about bases
    if (status->units[0].player == 'E'){
        *enemy_base = status->units[0];
        *our_base = status->units[1];
    }
    else {
        *enemy_base = status->units[1];
        *our_base = status->units[0];
    }

}

// Caclulate distances between reachable cells (for computation of vulnerability factor)
int* calculate_distances(struct map_state *map, int height, int width, struct status_info *status, int num_of_units){
    
    int* all_distances = allocate_2d_int_array(height * width, height * width);
    bool use_bfs_for_attack = false;

    for (int i = 0; i < height; i++){
        for (int j= 0; j < width; j++){
            
            int *current_distances = bfs(map, i, j, height, width, status, num_of_units, INT_MAX, use_bfs_for_attack);

            for (int k = 0; k < height * width; k++){
                all_distances[get_index_for_2d_array(i * width + j, k, height * width)] = current_distances[get_index_for_2d_array(k / width, k % width, width)];
            }
            free(current_distances);
        }
    }
    
    return all_distances;
}

// Breadth-first search algorithm, considering obstacles and units of the opponent
int* bfs(struct map_state *map, int row, int column, int height, int width, struct status_info *status, int num_of_units, int allowed_moves, bool bfs_for_attack){
    
    int* distances = (int*)malloc(height * width * sizeof(int));

    bool visited[height][width];

    // Filling visited elements with false
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            visited[i][j] = false;
        }
    }

    // Filling all array elements with -1
    for (int i = 0; i < height; i++){  
        for (int j = 0; j < width; j++){
            distances[i*width + j] = UNREACHABLE_DISTANCE;
        }
    }

    // Check if the cell is taken by an obstacle
    if (is_obstacle (row, column, map) || is_mine(row, column, map)){
        return distances;
    }

    int row_directions[] = {-1, 0, 1, 0};
    int column_directions[] = {0, 1, 0, -1};

    visited[row][column] = true;
    distances[get_index_for_2d_array(row, column, width)] = 0;

    // Create queue
    struct queue *que = create_queue();
    struct coordinates q_coord = {row, column};
    enqueue(que, q_coord);

    while(que->count > 0){

        struct coordinates cell_coord = dequeue(que);
        struct coordinates n = {0, 0};

        bool allowed_moves_reached = false;
        allowed_moves_reached = distances[cell_coord.x * width + cell_coord.y] >= allowed_moves;

        // Checking position adjusted horizontally or vertically
        for (int i = 0; i < 4; i++){

            if (!allowed_moves_reached){
                n.x = cell_coord.x + row_directions[i];
                n.y = cell_coord.y + column_directions[i];

                bool is_coord_taken = is_coordinates_taken(status, n.x, n.y, num_of_units);

                // Checking if current adjusted position is on the map, is not vistited before, and, if it's considered as obstacle, can be "jumped over"
                if (check_map_range(n.x, n.y, map) && (!visited[n.x][n.y])){
                    if (!((is_obstacle(n.x, n.y, map) || is_coord_taken || is_mine(n.x, n.y, map)) && (distances[cell_coord.x * width + cell_coord.y] + 1 > allowed_moves))){
                        distances[n.x * width + n.y] = distances[cell_coord.x * width + cell_coord.y] + 1;
                        visited[n.x][n.y] = true;
                        enqueue(que, n);
                    }
                }
            }
        }

        visited[cell_coord.x][cell_coord.y] = true;
    }

    free(que);

    // If counting bfs for attack 
    if (bfs_for_attack){
        for (int i = 0; i < height; i++){  
            for (int j = 0; j < width; j++){
                if (is_obstacle(i, j, map) || is_mine(i, j, map)){
                    distances[i * width + j] = UNREACHABLE_DISTANCE;
                }    
            }
        }
    }
    // If counting bfs for move
    else {
        for (int i = 0; i < height; i++){  
            for (int j = 0; j < width; j++){

                bool self_coord = false; 
                bool taken = false;
                if (is_coordinates_taken(status, i, j, num_of_units)){
                    taken = true;
                } 
                if (is_coordinates_taken(status, i, j, num_of_units) && i == row && j == column){
                    self_coord = true;
                }
                if (is_obstacle(i, j, map) || is_mine(i, j, map) || (taken && !self_coord)){
                        distances[i * width + j] = UNREACHABLE_DISTANCE;
                }    
            }
        }
    }

    return distances;
}

// Breadth-first search algorithm, considering obstacles and units of the opponent
int* bfs_for_workers(struct map_state *map, struct check_mine_result *result, int row, int column, int height, int width, struct status_info *status, int num_of_units, int allowed_moves){
    
    int* distances = allocate_2d_int_array(height, width);

    bool visited[height][width];

    // Filling visited elements with false
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            visited[i][j] = false;
        }
    }

    // Filling all array elements with -1
    for (int i = 0; i < height; i++){  
        for (int j = 0; j < width; j++){
            distances[i*width + j] = UNREACHABLE_DISTANCE;
        }
    }

    // Check if the cell is taken by an obstacle
    if (is_obstacle (row, column, map)){
        return distances;
    }

    int row_directions[] = {-1, 0, 1, 0};
    int column_directions[] = {0, 1, 0, -1};

    visited[row][column] = true;
    distances[get_index_for_2d_array(row, column, width)] = 0;

    // Create queue
    struct queue *que = create_queue();
    struct coordinates q_coord = {row, column};
    enqueue(que, q_coord);

    while(que->count > 0){

        struct coordinates cell_coord = dequeue(que);
        struct coordinates n = {0, 0};

        bool allowed_moves_reached = false;
        allowed_moves_reached = distances[cell_coord.x * width + cell_coord.y] >= allowed_moves;

        // Checking position adjusted horizontally or vertically
        for (int i = 0; i < 4; i++){
            if (!allowed_moves_reached){
                n.x = cell_coord.x + row_directions[i];
                n.y = cell_coord.y + column_directions[i];

                bool is_coord_taken = is_coordinates_taken(status, n.x, n.y, num_of_units);

                //Checking if current adjusted position is on the map, is not vistited before, and, if it's considered as obstacle, can be "jumped over"
                if (check_map_range(n.x, n.y, map) && (!visited[n.x][n.y])){
                    if (!((is_obstacle(n.x, n.y, map) || is_coord_taken) && (distances[cell_coord.x * width + cell_coord.y] + 1 > allowed_moves))){
                        distances[n.x * width + n.y] = distances[cell_coord.x * width + cell_coord.y] + 1;
                        visited[n.x][n.y] = true;
                        enqueue(que, n);
                    }
                }
            }
        }

        visited[cell_coord.x][cell_coord.y] = true;
    }

    for (int i = 0; i < height; i++){  
        for (int j = 0; j < width; j++){

            bool self_coord = false; 
            bool taken = false;
            bool our_unit_coord = false;

            if (is_coordinates_taken(status, i, j, num_of_units)){
                taken = true;
            } 
            if (is_coord_our_mine(i, j, result)){
                our_unit_coord = true;
            }
            if (i == row && j == column){
                self_coord = true;
            }
            if (is_obstacle(i, j, map) || (taken && !our_unit_coord && !self_coord)){
                    distances[i * width + j] = UNREACHABLE_DISTANCE;
            }    
        }
    }

    free(que);

    return distances;
}

bool is_coord_our_mine(int r, int c, struct check_mine_result *result){

    for (int i = 0; i < result->num_of_our_mines; i++){
        if (result->our_mines[i].x == r && result->our_mines[i].y == c){
            return true;
        }
    }
    return false;
}

// Count the total number of units
int get_total_number_of_units(char file_name[PATH_MAX]){

    // Subtract the first line with the gold data
    int units_counter = -1;

    // Load status file
    FILE *status_file;
    status_file = fopen(file_name, "r");
    char status_line[PATH_MAX];

    while(fgets(status_line, PATH_MAX, status_file)){
        units_counter++;
    }

    fclose(status_file);
    return units_counter; 
}

// Find the demage of an attack
int get_attack_damage(char attacker, char target, const struct unit_type* unittypes[]){

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
        {0, 0, 0, 0, 0, 0, 0, 0},
    };

    int attacker_index = get_unit_attack_index_by_notation(attacker, unittypes);
    int target_index = get_unit_attack_index_by_notation(target, unittypes);

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

// Functon to count the distance of a unit for speed and attack range
int get_distance(int x1, int x2, int y1, int y2) {
    int result = abs(x1 - x2) + abs(y1 - y2);
    return result;
}

// Check if the coordinates for the move is taken by other unit
bool is_coordinates_taken(struct status_info *status, int x, int y, int num_of_units){

    for(int i = 0; i < num_of_units; i++){
        if (status->units[i].coordinates.x == x && status->units[i].coordinates.y == y){
            return true;
        }
    }

    return false;
}

// Check if the coordinates for a move is within map range
bool check_map_range(int x, int y, struct map_state *map){
    return (x >= 0 && x < map->map_height && y >= 0 && y < map->map_width);
}

// Check if a mine on the coordinates
bool is_mine(int x, int y, struct map_state *map){
    for(int m = 0; m < map->num_of_mines; m++){
        if (map->mines[m].x == x && map->mines[m].y == y){
            return true;   
        }
    }
    return false;
}

// Check if obstacle on the coordinates
bool is_obstacle(int x, int y, struct map_state *map){

    for(int obst = 0; obst < map->num_of_obstacles; obst++){
        if ((map->obstacles[obst].x == x && map->obstacles[obst].y == y )&& (map->map[x][y] == '9')){
            return true;
        }
    }
    return false;
}

// Calculate the maximum attack demage for all unit types or for a particular one
void max_attack_demage(int index, bool all_types, int *demage){

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

    // Find max attack demage of a unit
    if (!all_types){
        for (int i = 0; i < NUM_OF_TYPES; i++){
            if (*demage < attack_table[index][i]){
                *demage = attack_table[index][i];
            }
        }
    }
    else {
        // Of all unit types
        for (int i = 0; i < NUM_OF_TYPES; i++){
            for (int j = 0; j < NUM_OF_TYPES; j++){
                if (*demage < attack_table[i][j]){
                    *demage = attack_table[i][j];
                }
            }
        }
    }
}

// Weightened strength for unit is the weightened strength of its unit type multiplied by the ratio of remaining health
double get_weightened_strength(struct status_info *status, struct unit un, const struct unit_type* unittypes[]){
    
    double weightened_strength_for_unit_type = get_weightened_strength_for_unit_type(un.unit_type, unittypes);
    double res = (un.current_strenght / (double)un.unit_type->strength) * weightened_strength_for_unit_type;
     
    return res;
}

// Weightenet strength of unit type is the sum of its attacking characteristics divided by the max respective values
double get_weightened_strength_for_unit_type(const struct unit_type* unit_t, const struct unit_type* unittypes[]){

    double max_strenght = 0;
    double velosity = 0;
    double max_range_attack = 0;
    int unit_max_demage = 0;
    int *ptr_m_d = &unit_max_demage;
    int max_attack_demage_of_all_units = 0;
    int *ptr_a = &max_attack_demage_of_all_units;

    // Find max value of the characteristics of all types
    for (int u = 0; u < NUM_OF_TYPES; u++){
        if (max_strenght < unittypes[u]->strength){
            max_strenght = (double)unittypes[u]->strength;
        }
        if (velosity < unittypes[u]->speed){
            velosity = (double)unittypes[u]->speed;
        }
        if (max_range_attack < unittypes[u]->range_of_attack){
            max_range_attack = (double)unittypes[u]->range_of_attack;
        }
    }

    int index = get_unit_attack_index_by_notation(unit_t->notation, unittypes);
    max_attack_demage(index, false, ptr_m_d);
    max_attack_demage(index, true, ptr_a);

    double weightened_strength = (double)(unit_t->strength / max_strenght) + (unit_t->speed / velosity) + (unit_t->range_of_attack / max_range_attack) + (unit_max_demage / max_attack_demage_of_all_units);
    
    return weightened_strength;
}

// Gets the distance between 2 positions in the map, considering obstacles e.t.c.
int get_reachable_distance(struct coordinates position1, struct coordinates position2, int height, int width, int* dist){
    
    int index = get_index_for_2d_array(position1.x * width + position1.y, position2.x * width + position2.y, height * width);

    return dist[index];
}

/*
Find vulnerability factor for each unit in relation to all other units.
Vulnerability factor is a coefficient to use heuristic to estimate the most appropriate unit to attack, based on the following considerations:
- we need to determine an enemy unit to attack in a way to cause max damage and get minimal counter attack damage, so we will attack the most isolated unit
- ror given enemy unit, as further it is located from the rest of "strong" enemies unit, as more isolated it is
- as closer it is located to our "strong" units, as more isolated it is
So, the isolation factor is the sum of each unit's relative strenght divided by number of turs to reach the given unit.
Each summand is positive when is of opponent's unit, and negative when is of the ally's one.
Resulted sum is multiplied by the remaining health of the given unit.
*/
void vulnerability_factor(struct status_info *status, struct map_state *map, const struct unit_type* un_types[], int height, int width, int* dist){

    // Target
    for (int i = 0; i < status->number_of_units; i++){

        // Attacker
        for (int j = 0; j < status->number_of_units; j++){

            if (i != j){
                
                status->units[j].weightened_strenght = get_weightened_strength(status, status->units[j], un_types);
                int reachable_distance = get_reachable_distance(status->units[j].coordinates, status->units[i].coordinates, height, width, dist);
                
                int num_of_turns_to_attack = (int)ceil(reachable_distance - (double)(status->units[j].unit_type->range_of_attack) / (double)(status->units[j].unit_type->speed));

                if (num_of_turns_to_attack < 1){
                    num_of_turns_to_attack = 1;
                }
                // If both units are from one player
                int additional_val = 0;
                if (status->units[i].player == status->units[j].player){
                    additional_val = -1;
                }
                else {
                    additional_val = 1;
                }

                // Calculating the vulnerability factor
                status->units[i].vulnerability_factor += (additional_val * status->units[j].weightened_strenght / num_of_turns_to_attack);
            }
        }
    }
}

void to_empty_orders_file(char file_name[PATH_MAX]){
    FILE *f;
    f = fopen(file_name, "w" ); 
    fclose(f);
}

// Write the orders into a file
bool write_move_order(char file_name[PATH_MAX], int id, int x, int y){

    FILE *orders_f;
    orders_f = fopen(file_name, "a");

    char *move_result = (char*)malloc(4 * sizeof(int));
    char new_line[] = "\n";

    // Pattern
    char move_unit[30] = "%u M %u %u";

    sprintf(move_result, move_unit, id, x, y);

    fwrite(move_result, 1, strlen(move_result), orders_f);
    fwrite(new_line, 1, strlen(new_line), orders_f);

    fclose(orders_f);
    free(move_result);

    return true;
}

bool write_attack_order(char file_name[PATH_MAX], int id_attacker, int id_target){

    FILE *orders_f;
    orders_f = fopen(file_name, "a");

    char *attack_result = (char*)malloc(3 * sizeof(int));
    char attack_unit[20] = "%u A %u";
    char new_line[] = "\n";

    sprintf(attack_result, attack_unit, id_attacker, id_target);

    fwrite(attack_result, 1, strlen(attack_result), orders_f);
    fwrite(new_line, 1, strlen(new_line), orders_f);

    fclose(orders_f);
    free(attack_result);

    return true;
}

bool write_build_order(char file_name[PATH_MAX], int base_id, char type){

    FILE *orders_f;
    orders_f = fopen(file_name, "a");

    char *build_result = (char*)malloc(2 * sizeof(int));
    char build_unit[20] = "%u B %c";
    char new_line[] = "\n";

    sprintf(build_result, build_unit, base_id, type);

    fwrite(build_result, 1, strlen(build_result), orders_f);
    fwrite(new_line, 1, strlen(new_line), orders_f);

    fclose(orders_f);
    free(build_result);

    return true;
}

// Find a unit to use as target near the given mine
struct unit find_target_near_mine(struct check_mine_result *mines_data,  struct status_info *status, int num_of_units){

    double max_vuln = INT_MAX;
    int id = 0;

    for (int i = 0; i < mines_data->num_of_enemy_mines; i++){
        for(int j = 0; j < num_of_units; j++){
            if (mines_data->enemy_mines[i].x == status->units[j].coordinates.x && mines_data->enemy_mines[i].y == status->units[j].coordinates.y && status->units[j].player == 'E'){
                if (status->units[j].vulnerability_factor < max_vuln){
                    max_vuln = status->units[j].vulnerability_factor;
                    id = j;
                }
            }
        }
    }

    return status->units[id];
}

bool send_workers_to_mines(struct check_mine_result *mines_data, struct map_state *map, struct status_info *status, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[PATH_MAX]){

    int units[num_of_units];
    bool enemy_unit = true;
    int number_to_find_unit_in_status = 0;
    bool available_worker = false;

    int* distance;
    struct unit* worker = (struct unit*) malloc(sizeof(struct unit));

    // Find available worker
    for(int i = 0; i < num_of_units; i++){
        if (status->units[i].unit_type->notation == 'W' && status->units[i].player != 'E' && (!is_mine(status->units[i].coordinates.x, status->units[i].coordinates.y, map))){
            *worker = status->units[i];
            available_worker = true;
            number_to_find_unit_in_status = i;
            distance = bfs_for_workers(map, mines_data, status->units[i].coordinates.x, status->units[i].coordinates.y, map->map_height, map->map_width, status, num_of_units, worker->remaining_moves);
            break;
        }
    }

    if (!available_worker){
        free(worker);
        return false;
    }

    // If there are no available mines
    if (mines_data->num_of_free_mines < 1 && mines_data->num_of_our_mines < 1 && mines_data->num_of_enemy_mines > 1){

        // Find a target near the enemy mine
        struct unit* target = (struct unit*) malloc(sizeof(struct unit));
        *target = find_target_near_mine(mines_data, status, num_of_units);

        // Find units to attack the target
        find_units(status, target, units, num_of_units, unittypes, enemy_unit);
        int available_units = count_available_units(units, num_of_units);

        struct unit* attacker = (struct unit*) malloc(sizeof(struct unit));

        // Attack the target
        for (int i = 0; i < available_units; i++){
            if (target->current_strenght > 0){
                *attacker = status->units[units[i]];
                attack_unit(map, status, attacker, target, dist, result, num_of_units, unittypes, file_name);
                status->units[units[i]] = *attacker; 
            }
        }

        free(worker);
        free(target);
        free(attacker);
        free(distance);
    }
    // If some mines are available
    else {

        struct coordinates* cell_to_move_worker = (struct coordinates*) malloc(sizeof(struct coordinates));
        int min_distance = INT_MAX;
        int id = 0;
        int temp_index;

        // If we alredy have our mines
        if (mines_data->num_of_our_mines > 0){
            // Find the closest mine
            for (int m = 0; m < mines_data->num_of_our_mines; m++){

                temp_index = get_index_for_2d_array(mines_data->our_mines[m].x, mines_data->our_mines[m].y, map->map_width);

                if (distance[temp_index] != UNREACHABLE_DISTANCE && distance[temp_index] < min_distance){
                    min_distance = distance[temp_index];
                    id = m;
                }
            }

            *cell_to_move_worker = mines_data->our_mines[id];
        }
        // If there are not taken mines 
        else if (mines_data->num_of_free_mines > 0){
            for (int m = 0; m < mines_data->num_of_free_mines; m++){

                temp_index = get_index_for_2d_array(mines_data->free_mines[m].x, mines_data->free_mines[m].y, map->map_width);

                if (distance[temp_index] != UNREACHABLE_DISTANCE && distance[temp_index] < min_distance){
                    min_distance = distance[temp_index];
                    id = m;
                }
            }

            *cell_to_move_worker = mines_data->free_mines[id];
        }
        else {
            free(cell_to_move_worker);
            free(distance);
            free(worker);
            return false;
        }

        // Calculate distance
        int count_dist = get_distance(worker->coordinates.x, cell_to_move_worker->x, worker->coordinates.y, cell_to_move_worker->y);

        // Coordinates for the available cell if cell_to_move_worker is too far away
        struct coordinates* available_cell = (struct coordinates*) malloc(sizeof(struct coordinates));

        // If cell is unreachable
        if (count_dist > worker->remaining_moves){
            // Find an available cell
            closest_cell_worker(map, status, mines_data, num_of_units, worker->coordinates.x, worker->coordinates.y, worker->remaining_moves, cell_to_move_worker->x, cell_to_move_worker->y, dist, available_cell);
            count_dist = get_distance(worker->coordinates.x, available_cell->x, worker->coordinates.y, available_cell->y);
        
            // Move unit closer to the mine
            if (count_dist <= worker->remaining_moves){
                worker->coordinates.x = available_cell->x;
                worker->coordinates.y =  available_cell->y;
                worker->remaining_moves -= count_dist;
            }
        }

        // Move unit to the mine
        else if (count_dist <= worker->remaining_moves){
            worker->coordinates.x = cell_to_move_worker->x;
            worker->coordinates.y =  cell_to_move_worker->y;
            worker->remaining_moves -= count_dist;
        }

        // Save the update in status
        status->units[number_to_find_unit_in_status] = *worker;

        // Create a new move order
        write_move_order(file_name, worker->unit_id, worker->coordinates.x, worker->coordinates.y);

        free(worker);
        free(distance);
        free(available_cell);  
        free(cell_to_move_worker);
        return true;
    }

    free(distance);
    free(worker);
    return false;
}

bool attack_enemy_base(struct map_state *map, struct status_info *status, struct unit* base, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[PATH_MAX]){

    int units[num_of_units];
    bool enemy_base = true;
    int available_units = 0;

    struct unit* attacker = (struct unit*) malloc(sizeof(struct unit));
    struct unit* target = (struct unit*) malloc(sizeof(struct unit));

    // If target is found 
    if (find_target_near_given_base(status, base, target, num_of_units, unittypes, enemy_base)){

        // Find available units
        if (!find_units(status, target, units, num_of_units, unittypes, enemy_base)){
            free(target);
            free(attacker);
            return false;
        }

        available_units = count_available_units(units, num_of_units);

        // Attack target
        for (int i = 0; i < available_units; i++){
            if (target->current_strenght > 0){
                *attacker = status->units[units[i]];
                attack_unit(map, status, attacker, target, dist, result, num_of_units, unittypes, file_name);
                status->units[units[i]] = *attacker; 
            }
        }

        // Attack the enemy base if previous target has been destroyed
        if (target->current_strenght <= 0){

            // Find available units
            if (!find_units(status, target, units, num_of_units, unittypes, enemy_base)){
                free(target);
                free(attacker);
                return false;
            }

            available_units = count_available_units(units, num_of_units);

            for (int i = 0; i < available_units; i++){
                if (base->current_strenght > 0){
                    *attacker = status->units[units[i]];
                    attack_unit(map, status, attacker, base, dist, result, num_of_units, unittypes, file_name);
                    status->units[units[i]] = *attacker; 
                }
            }
        }
    }
    // If there is no enemy units near the base
    else {
        // Find available units
        if (!find_units(status, target, units, num_of_units, unittypes, enemy_base)){
            free(target);
            free(attacker);
            return false;
        }

        available_units = count_available_units(units, num_of_units);

        // Attack the enemy base with each of the units
        for (int i = 0; i < available_units; i++){
            if (base->current_strenght > 0){
                *attacker = status->units[units[i]];
                attack_unit(map, status, attacker, base, dist, result, num_of_units, unittypes, file_name);
                status->units[units[i]] = *attacker; 
            }
        }
    }
    
    free(target);
    free(attacker);
    return true;
}

bool attack_unit(struct map_state *map, struct status_info *status, struct unit *attacker, struct unit *target, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[PATH_MAX]){

    bool attacked = false;
    attacker->busy = true;

    // Try to attack the target
    attacked = attempt_to_attack_unit(map, status, attacker, target, dist, num_of_units, unittypes, file_name);

    // If attempt to attack was successful
    if (attacked){
        return true;
    }

    // Otherwise move unit closer to the target 
    struct coordinates* cell_to_move_attacker = (struct coordinates*) malloc(sizeof(struct coordinates));
    bool is_worker = false;

    if (target->unit_type->notation == 'W'){
        is_worker = true;
        get_closest_cell(map, status, result, num_of_units, attacker->coordinates.x, attacker->coordinates.y, attacker->remaining_moves, target->coordinates.x, target->coordinates.y, dist, cell_to_move_attacker, is_worker);
    }
    else {
        get_closest_cell(map, status, result, num_of_units, attacker->coordinates.x, attacker->coordinates.y, attacker->remaining_moves, target->coordinates.x, target->coordinates.y, dist, cell_to_move_attacker, is_worker);
    }

    // Calculate the available distance to move
    int distance = get_distance(attacker->coordinates.x, cell_to_move_attacker->x, attacker->coordinates.y, cell_to_move_attacker->y);

    // If move to the closest cell near the enemy is posiible
    if (distance <= attacker->remaining_moves){
        attacker->coordinates.x = cell_to_move_attacker->x;
        attacker->coordinates.y =  cell_to_move_attacker->y;
        attacker->remaining_moves -= distance;

        // Create move order
        write_move_order(file_name, attacker->unit_id, attacker->coordinates.x, attacker->coordinates.y);
    } 

    // Try to attack again
    attacked = attempt_to_attack_unit(map, status, attacker, target, dist, num_of_units, unittypes, file_name);

    if (!attacked){
        attacker->busy = false;
    }

    free(cell_to_move_attacker);
    return attacked;
}

bool attempt_to_attack_unit(struct map_state *map, struct status_info *status, struct unit *attacker, struct unit *target, int* dist, int num_of_units, const struct unit_type* unittypes[], char file_name[PATH_MAX]){

    bool bfs_for_attack = true;
    int *attack_dist = bfs(map, attacker->coordinates.x, attacker->coordinates.y, map->map_height, map->map_width, status, num_of_units, attacker->unit_type->range_of_attack, bfs_for_attack);
    int index = get_index_for_2d_array(target->coordinates.x, target->coordinates.y, map->map_width);
    
    // If can attack right away
    if (attack_dist[index] != UNREACHABLE_DISTANCE){

        int demage = get_attack_damage(attacker->unit_type->notation, target->unit_type->notation, unittypes);
        
        while (attacker->remaining_moves > 0){
            target->current_strenght = target->current_strenght - demage;
            attacker->remaining_moves--;

            // Create a new attack order
            write_attack_order(file_name, attacker->unit_id, target->unit_id);

            if (target->current_strenght <= 0){
                attacker->busy = false;
                free(attack_dist);
                return true;
            }
        }
    }

    free(attack_dist);
    return false;
}

// The function determines the closest pair of cells of 2 separated areas on the map.
// It's based on "brute force" getting through each pair of points and should be replaced with another algorithm for closest points search
struct coordinates* get_closest_cell(struct map_state *map, struct status_info *status, struct check_mine_result *result,  int num_of_units, int row1, int col1, int steps_left, int row2, int col2, int* dist, struct coordinates* cell, bool is_worker){

    bool bfs_for_attack = false;
    int* distance_attacker = bfs(map, row1, col1, map->map_height, map->map_width, status, num_of_units, steps_left, bfs_for_attack);
    
    int* distance_target;
    
    if (is_worker){
        distance_target = bfs_for_workers(map, result, row2, col2, map->map_height, map->map_width, status, num_of_units, steps_left);
    }
    else {
        distance_target = bfs(map, row2, col2, map->map_height, map->map_width, status, num_of_units, steps_left, bfs_for_attack);
    }

    struct coordinates *new_position_1 = (struct coordinates*) malloc(sizeof(struct coordinates));
    struct coordinates *new_position_2 = (struct coordinates*) malloc(sizeof(struct coordinates));

    int closest_row = -1;
    int closest_col = -1;
    int min_dist = INT_MAX;
    int i = 0;

    while (i < map->map_height){
        for (int j = 0; j < map->map_width; j++){
            for (int k = 0; k < map->map_height; k++){
                for (int l = 0; l < map->map_width; l++){
                    new_position_1->x = i; 
                    new_position_1->y = j;
                    new_position_2->x = k;
                    new_position_2->y = l;

                    // Get reachable distance considering obstacles
                    int reachable_dist = get_reachable_distance(*new_position_1, *new_position_2, map->map_height, map->map_width, dist);

                    if ((distance_attacker[i * map->map_width + j] != UNREACHABLE_DISTANCE) && (distance_target[k * map->map_width + l] != UNREACHABLE_DISTANCE) && reachable_dist <= min_dist && reachable_dist != UNREACHABLE_DISTANCE){
                        min_dist = reachable_dist;
                        closest_row = i;
                        closest_col = j;
                    }
                }
            }
        }

        i++;
    }

    cell->x = closest_row;
    cell->y = closest_col;

    free(distance_attacker);
    free(distance_target);
    free(new_position_1);
    free(new_position_2);

    return cell;
}

// Get the closest cell to move a unit
struct coordinates* closest_cell_worker(struct map_state *map, struct status_info *status, struct check_mine_result *result, int num_of_units, int row1, int col1, int steps_left, int row2, int col2, int* dist, struct coordinates* cell){

    int* distance_worker = bfs_for_workers(map, result, row1, col1, map->map_height, map->map_width, status, num_of_units, steps_left);
    int* distance_cell = bfs_for_workers(map, result, row2, col2, map->map_height, map->map_width, status, num_of_units, steps_left);

    struct coordinates *new_position_1 = (struct coordinates*) malloc(sizeof(struct coordinates));
    struct coordinates *new_position_2 = (struct coordinates*) malloc(sizeof(struct coordinates));

    int closest_row = -1;
    int closest_col = -1;
    int min_dist = INT_MAX;
    int i = 0;

    while (i < map->map_height){
        for (int j = 0; j < map->map_width; j++){
            for (int k = 0; k < map->map_height; k++){
                for (int l = 0; l < map->map_width; l++){
                    new_position_1->x = i; 
                    new_position_1->y = j;
                    new_position_2->x = k;
                    new_position_2->y = l;

                    // Get reachable distance considering obstacles
                    int reachable_dist = get_reachable_distance(*new_position_1, *new_position_2, map->map_height, map->map_width, dist);

                    if ((distance_worker[i * map->map_width + j] != UNREACHABLE_DISTANCE) && (distance_cell[k * map->map_width + l] != UNREACHABLE_DISTANCE) && reachable_dist <= min_dist && reachable_dist != UNREACHABLE_DISTANCE){
                        min_dist = reachable_dist;
                        closest_row = i; // was k, l
                        closest_col = j;
                    }
                }
            }
        }

        i++;
    }

    cell->x = closest_row;
    cell->y = closest_col;

    free(distance_worker);
    free(distance_cell);
    free(new_position_1);
    free(new_position_2);

    return cell;
}

// Find a unit to use as target near the given base
bool find_target_near_given_base(struct status_info *status, struct unit *base, struct unit *target, int num_of_units, const struct unit_type* unittypes[], bool enemy_base){

    // Find max attack range for the unit
    bool all_types = false;
    int max_range_attack = 0;;
    int index = 0;
    int *dem = &max_range_attack;

    double protection_factor = 0;
    double attack_factor = 0;
    double param = INT_MIN;
    double compare_param = 0;
    int id = -1;
    
    // Find all enemies near the base
    for (int i = 0; i < num_of_units; i++){

        int distance = get_distance(base->coordinates.x, status->units[i].coordinates.x, base->coordinates.y, status->units[i].coordinates.y);
        
        if (status->units[i].player == 'E' && status->units[i].current_strenght > 0 && status->units[i].unit_type->notation != 'B'){

            index = get_unit_attack_index_by_notation(status->units[i].unit_type->notation, unittypes);
            max_attack_demage(index, all_types, dem);

            if (distance <= max_range_attack){

                if (enemy_base){
                    // Base protection factor is a weightened damage unit cause to main base assaulting units: catapult and ram
                    protection_factor = (get_attack_damage(status->units[i].unit_type->notation, 'C', unittypes) + get_attack_damage(status->units[i].unit_type->notation, 'R', unittypes)) / (double)(2 * max_range_attack);
                    compare_param = (get_weightened_strength(status, status->units[i], unittypes) * protection_factor) / distance;  
                }
                else {
                    // Base attach factor is relative damage unit cause to base
                    attack_factor = (double)get_attack_damage(status->units[i].unit_type->notation, 'B', unittypes) / max_range_attack;
                    compare_param = (get_weightened_strength(status, status->units[i], unittypes) * attack_factor / distance);
                }

                // Get the unit with the max parameter
                if (compare_param > param){
                    param = compare_param;
                    id = i;
                }
            }
        }
    }

    if (id <= 0){
        return false;
    }

    *target = status->units[id];
    return true; 
}

// Find all units to attack
bool find_units(struct status_info *status, struct unit *unit, int units[], int num_of_units,  const struct unit_type* unittypes[], bool enemy_unit){
    
    double attack_factor = 0;
    double protection_factor = 0;
    double compare_param;
    int counter = 0;
    bool all_types = false;

    struct id_param* units_param = (struct id_param*) malloc(num_of_units * sizeof(struct id_param));
    
    // Clean the units arr
    for (int u = 0; u < num_of_units; u++){
        units[u] = -1;
    }

    // Find all units who is not of type 'worker' and who is not busy 
    for (int i = 0; i < num_of_units; i++){

        if (status->units[i].player == 'P' && (!status->units[i].busy) && status->units[i].unit_type->notation != 'W' && status->units[i].unit_type->notation != 'B' && status->units[i].remaining_moves > 0 && status->units[i].current_strenght > 0){
            
            int distance = get_distance(status->units[i].coordinates.x, unit->coordinates.x, status->units[i].coordinates.y, unit->coordinates.y);
            
            // Find max attack range of all types
            int index = 1;
            int max_demage = 0;
            int *ptr_dem = &max_demage;
            max_attack_demage(index, all_types, ptr_dem);

            // Search for units to attack an enemy unit 
            if (enemy_unit){
                attack_factor = (double)get_attack_damage(status->units[i].unit_type->notation, 'B', unittypes) / max_demage;
                compare_param = (get_weightened_strength(status, status->units[i], unittypes) * attack_factor) / distance;
            }
            // Search for units to protect our own unit
            else {
                protection_factor = (get_attack_damage(status->units[i].unit_type->notation, 'C', unittypes) + get_attack_damage(status->units[i].unit_type->notation, 'R', unittypes)) / (double)(2 * max_demage);
                compare_param = (get_weightened_strength(status, status->units[i], unittypes) * protection_factor) / distance;
            }

            units_param[counter].id = i;
            units_param[counter].param = compare_param;
            counter++;
        }
    }

    // If no available units
    if (counter == 0){
        free(units_param);
        return false;
    }

    // Sort the units in descending order
    qsort(units_param, counter, sizeof(struct id_param), compare_desc);

    // Write the units ids into units[]
    for (int i = 0; i < counter; i++){
        units[i] = units_param[i].id;
    }

    free(units_param);
    return true;

}

// Comparer function for the quicksort
int compare_desc(const void* p1, const void* p2){
    
    struct id_param* param1 = (struct id_param*) p1;
    struct id_param* param2 = (struct id_param*) p2;

    if (param1->param > param2->param ){
        return -1;
    }
    else if (param1->param < param2->param){
        return 1;
    }

    return 0;
}

bool build_new_unit(struct status_info *status, struct unit *base, const struct unit_type* unittypes[], char file_name[PATH_MAX]){

    if (base->unit_type_being_built != '0' || status->gold <= 100){
        return false;
    }

    char notation;
    bool attack = true;
    
    // If the amount of gold is less then 300, build a worker
    if (status->gold <= 400){
        notation = 'W';
    }
    // Find a needed unit type to protect the base if base is in danger
    else if (status->is_base_in_danger && status->gold >= 800){
        attack = false;
        notation = get_unit_type_priority(status, status->gold, unittypes, attack);
    }
    // Find the unit type needed to attack the enemys base
    else if (!status->is_base_in_danger && status->gold >= 800){
        notation = get_unit_type_priority(status, status->gold, unittypes, attack);
    }
    else if (status->gold < 800){
        notation = 'K';
    }

    // Write new order to a file
    write_build_order(file_name, base->unit_id, notation);

    return true;
}

/*
Get the type notation to order to buid
This function calculates a simple heuristic value function to define the preferable unit type to build,
considering the current amount of gold, unit's relative strenght, unit's build cost and time to build,
and our current high priority goal (protect our base, attack enemy's base or mine more gold)
*/
char get_unit_type_priority(struct status_info *status, int gold, const struct unit_type* unittypes[], bool attack){

    double priority = INT_MIN;
    double temp_type_priority;
    int id = 0;

    // Find max attack demage of a unit type
    bool all_types = false;
    int max_range_attack = 0;
    int index = 0;
    int *dem = &max_range_attack;

    for (int i = 0; i < NUM_OF_TYPES; i++){

        if (unittypes[i]->notation != 'B'){

            index = get_unit_attack_index_by_notation(unittypes[i]->notation, unittypes);
            max_attack_demage(index, all_types, dem);

            // Get the unit types weightened strenght
            double weightened_strength = get_weightened_strength_for_unit_type(unittypes[i], unittypes);

            // If searching for a unit type for protection
            if (!attack){
                double protection_factor = (get_attack_damage(unittypes[i]->notation, 'C', unittypes) + get_attack_damage(unittypes[i]->notation, 'R', unittypes)) / (double)(2 * max_range_attack);
                temp_type_priority = weightened_strength * max_range_attack * protection_factor * (gold - unittypes[i]->cost) / (unittypes[i]->time_to_build * unittypes[i]->cost); 
            }
            // If searching for a unit type for attack 
            else {
                double attack_factor = (double)get_attack_damage(unittypes[i]->notation, 'B', unittypes) / max_range_attack;
                temp_type_priority = weightened_strength * attack_factor * (double)get_attack_damage(unittypes[i]->notation, 'B', unittypes) * unittypes[i]->speed * unittypes[i]->range_of_attack * (gold - unittypes[i]->cost) / (unittypes[i]->time_to_build * unittypes[i]->cost);
            }

            // Get the unit id with the max priority
            if (priority < temp_type_priority){
                priority = temp_type_priority;
                id = i;
            }
        }
    }

    return unittypes[id]->notation;
}

double get_relative_strength_balance_near_our_base(struct status_info *status, struct unit* base, int num_of_units, const struct unit_type* unittypes[]){

    double balance = 0;

    // Find max attack range of all types
    bool all_types = true;
    int max_range_attack = 0;
    int index = 1;
    int *dem = &max_range_attack;
    max_attack_demage(index, all_types, dem);

    // Count relative strenght balance
    for (int i = 0; i < num_of_units; i++){

        int distance = get_distance(base->coordinates.x, status->units[i].coordinates.x, base->coordinates.y, status->units[i].coordinates.y);
        
        if (distance <= max_range_attack){
            if (status->units[i].player == 'E'){
                double attack_factor = (double)get_attack_damage(status->units[i].unit_type->notation, 'B', unittypes) / (double)max_range_attack;
                balance += (-attack_factor) * status->units[i].weightened_strenght;
            }
            else {
                double protection_factor = (get_attack_damage(status->units[i].unit_type->notation, 'C', unittypes) + get_attack_damage(status->units[i].unit_type->notation, 'R', unittypes)) / (double)(2.0 * max_range_attack);
                balance += protection_factor * status->units[i].weightened_strenght;
            }
        }
    }

    return balance;
}

bool protect_our_base(struct map_state *map, struct status_info *status, struct unit* base, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[PATH_MAX]){

    double relative_strength_balance = 0.0;
    relative_strength_balance = get_relative_strength_balance_near_our_base(status, base, num_of_units, unittypes);
    
    // If base not in danger
    if (relative_strength_balance > 0){
        return false;
    }

    // Find units to protect our base
    int units[num_of_units];
    bool enemy_unit = false;

    if (!find_units(status, base, units, num_of_units, unittypes, enemy_unit)){
        return false;
    }
    
    int available_units = count_available_units(units, num_of_units);

    // Find a target
    struct unit* target = (struct unit*) malloc(sizeof(struct unit));

    if (!find_target_near_given_base(status, base, target, num_of_units, unittypes, enemy_unit)){
        free(target);
        return false;
    }

    struct unit* attacker = (struct unit*) malloc(sizeof(struct unit));

    // Attack target
    for (int i = 0; i < available_units; i++){

        if (target->current_strenght > 0){
            relative_strength_balance = get_relative_strength_balance_near_our_base(status, base, num_of_units, unittypes);

            if (relative_strength_balance < 0){
                *attacker = status->units[units[i]];
                attack_unit(map, status, attacker, target, dist, result, num_of_units, unittypes, file_name);
                status->units[units[i]] = *attacker; 
            }
        }
        else {
            break;
        }
    }

    relative_strength_balance = get_relative_strength_balance_near_our_base(status, base, num_of_units, unittypes);

    if (relative_strength_balance < 0){
        status->is_base_in_danger = true;
    }

    free(target);
    free(attacker);

    return true;
}

// Count units who are available to do actions
int count_available_units(int units[], int num_of_units){

    int count = 0;

    for (int u = 0; u < num_of_units; u++){
        if (units[u] != -1){
            count++;
        }
    }

    return count;
}

// Helper functions
int* allocate_2d_int_array(int num_of_rows, int num_of_columns){

    return (int*)malloc(num_of_rows * num_of_columns * sizeof(int));
}

int get_index_for_2d_array(int row, int col, int num_of_columns)
{
    return row * num_of_columns + col;
}

void free_memory(struct map_state *map, struct status_info *status, struct unit* e_base, struct unit* our_base, int* dist, struct check_mine_result *result, struct unit* units){

    free(dist);
    free(our_base);
    free(e_base);
    free(units);
    free(status);
    free(map);
    free(result);
}