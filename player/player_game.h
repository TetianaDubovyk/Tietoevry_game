#ifndef GAME_HEAD_H
#define GAME_HEAD_H

#include <stdlib.h>

#define MAX_STRING_SIZE 40
#define MAX_SIZE_OF_MAP 100
#define NUM_OF_TYPES 8
#define MAX_MINES 50
#define MAX_OBSTACLES 100
#define MAX_NUM_OF_WORKERS_IN_MINES 100
#define UNREACHABLE_DISTANCE -1


struct coordinates {
    int x;
    int y;
};

struct unit_type {
    char notation;
    int strength;
    int speed;
    int cost;
    int range_of_attack;
    int time_to_build;
};

struct unit {
    char player, unit_type_being_built;
    int unit_id, current_strenght, time_to_build;
    const struct unit_type* unit_type;
    struct coordinates coordinates; 
    bool busy;
    int remaining_moves;
    double vulnerability_factor;
    double weightened_strenght;
};

struct status_info {
    int gold;
    struct unit* units;
    int number_of_units;
    bool is_base_in_danger;
};

struct map_state {
    char map[MAX_SIZE_OF_MAP][MAX_SIZE_OF_MAP];
    int map_width, map_height;
    char player1_base, player2_base;
    struct coordinates bases[2]; // both bases
    struct coordinates mines[MAX_MINES];
    int num_of_mines;
    int num_of_obstacles;
    struct coordinates obstacles[MAX_OBSTACLES];
};

struct check_mine_result {
    struct coordinates free_mines[MAX_MINES];
    int num_of_mining_workers;
    struct coordinates our_mines[MAX_MINES];
    struct coordinates enemy_mines[MAX_MINES];
    int num_of_enemy_mines;
    int num_of_our_mines;
    int num_of_free_mines;
};

struct id_param {
    int id;
    double param;
};


const struct unit_type knight = {'K', 70, 5, 400, 1, 5};
const struct unit_type swordsman = {'S', 60, 2, 250, 1, 3};
const struct unit_type archer = {'A', 40, 2, 250, 5, 3};
const struct unit_type pikeman = {'P', 50, 2, 200, 2, 3};
const struct unit_type ram = {'R', 90, 2, 500, 1, 4};
const struct unit_type catapult = {'C', 50, 2, 800, 7, 6};
const struct unit_type worker = {'W', 20, 2, 100, 1, 2};
const struct unit_type base = {'B', 200, 0};


int* allocate_2d_int_array(int num_of_rows, int num_of_columns);
int get_index_for_2d_array(int row, int col, int num_of_columns);
bool load_status (char file_name[MAX_STRING_SIZE], struct status_info *status, struct unit* units);
bool load_map (char file_name[MAX_STRING_SIZE], struct map_state *map);
void check_mines(struct map_state *map, struct status_info *status, struct check_mine_result *result, int num_of_units);
void update_gold(struct status_info *status, struct check_mine_result *result);
void find_bases(struct status_info *status, struct unit* our_base, struct unit* enemy_base);
int* calculate_distances(struct map_state *map, int height, int width, struct status_info *status, int num_of_units);
int* bfs(struct map_state *map, int row, int column, int height, int width, struct status_info *status, int num_of_units, int allowed_moves, bool bfs_for_attack);
int* bfs_for_workers(struct map_state *map, struct check_mine_result *result, int row, int column, int height, int width, struct status_info *status, int num_of_units, int allowed_moves);
bool is_coordinates_taken(struct status_info *status, int x, int y, int num_of_units);
bool check_map_range(int x, int y, struct map_state *map_ptr);
bool is_coord_our_mine(int r, int c, struct check_mine_result *result);
bool is_mine(int x, int y, struct map_state *map);
bool is_base(int x, int y, struct map_state *map);
bool is_obstacle(int x, int y, struct map_state *map);
void max_attack_demage(int index, bool all_types, int *demage);
int get_total_number_of_units(char file_name[MAX_STRING_SIZE]);
int get_unit_attack_index_by_notation(char unit_notation, const struct unit_type* unittypes[]);
int get_attack_damage(char attacker, char target, const struct unit_type* unittypes[]);
int get_distance(int x1, int x2, int y1, int y2);
char get_unit_type_priority(struct status_info *status, int gold, const struct unit_type* unittypes[], bool attack);
int get_reachable_distance(struct coordinates unit1, struct coordinates unit2, int height, int width, int* dist);
double get_relative_strength_balance_near_our_base(struct status_info *status, struct unit* base, int num_of_units, const struct unit_type* unittypes[]);
struct coordinates* get_closest_cell(struct map_state *map, struct status_info *status, struct check_mine_result *result, int num_of_units, int row1, int col1, int steps_left, int row2, int col2, int* dist, struct coordinates* cell, bool is_worker);
struct coordinates* closest_cell_worker(struct map_state *map, struct status_info *status, struct check_mine_result *result, int num_of_units, int row1, int col1, int steps_left, int row2, int col2, int* dist, struct coordinates* cell);
double get_weightened_strength(struct status_info *status, struct unit un, const struct unit_type* unittypes[]);
double get_weightened_strength_for_unit_type(const struct unit_type* unit_t, const struct unit_type* unittypes[]);
void vulnerability_factor(struct status_info *status, struct map_state *map, const struct unit_type* un_types[], int height, int width, int* dist);
bool send_workers_to_mines(struct check_mine_result *mines_data, struct map_state *map, struct status_info *status, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[MAX_STRING_SIZE]);
bool attack_unit(struct map_state *map, struct status_info *status, struct unit* attacker, struct unit* target, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[MAX_STRING_SIZE]);
bool find_target_near_given_base(struct status_info *status, struct unit *base, struct unit* target, int num_of_units, const struct unit_type* unittypes[], bool enemy_base);
bool find_units(struct status_info *status, struct unit *unit, int units[], int num_of_units,  const struct unit_type* unittypes[], bool enemy_unit);
bool protect_our_base(struct map_state *map, struct status_info *status, struct unit* base, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[MAX_STRING_SIZE]);
bool build_new_unit(struct status_info *status, struct unit *base, const struct unit_type* unittypes[], char file_name[MAX_STRING_SIZE]);
bool attack_enemy_base(struct map_state *map, struct status_info *status, struct unit* base, int* dist, struct check_mine_result *result, int num_of_units, const struct unit_type* unittypes[], char file_name[MAX_STRING_SIZE]);
bool attempt_to_attack_unit(struct map_state *map, struct status_info *status, struct unit* attacker, struct unit* target, int* dist, int num_of_units, const struct unit_type* unittypes[], char file_name[MAX_STRING_SIZE]);
int count_available_units(int units[], int num_of_units);
int compare_desc(const void* p1, const void* p2);
void to_empty_orders_file(char file_name[MAX_STRING_SIZE]);
bool write_move_order(char file_name[MAX_STRING_SIZE], int id, int x, int y);
bool write_attack_order(char file_name[MAX_STRING_SIZE], int id_attacker, int id_target);
bool write_build_order(char file_name[MAX_STRING_SIZE], int base_id, char type);


#endif
