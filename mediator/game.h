#ifndef GAME_HEAD_H
#define GAME_HEAD_H


#define MAX_STRING_SIZE 20
#define MAX_SIZE_OF_MAP 50
#define NUM_OF_TYPES 8
#define UNREACHABLE_DISTANCE -1
#define MAX_MINES 10
#define MAX_OBSTACLES 100
#define P "Player #1"


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
    int remaining_moves;
};

struct status_info {
    int player1_gold, player2_gold;
    struct unit* units;
    int player1_workers_in_mine, player2_workers_in_mine;
    int number_of_units_player1, number_of_units_player2;
};

struct map_state {
    char map[MAX_SIZE_OF_MAP][MAX_SIZE_OF_MAP];
    int map_width, map_height;
    char player1_base, player2_base;
    struct coordinates bases[2]; // both bases
    struct coordinates mines[MAX_MINES];
    struct coordinates obstacles[MAX_OBSTACLES];
};

const struct unit_type knight = {'K', 70, 5, 400, 1, 5};
const struct unit_type swordsman = {'S', 60, 2, 250, 1, 3};
const struct unit_type archer = {'A', 40, 2, 250, 5, 3};
const struct unit_type pikeman = {'P', 50, 2, 200, 2, 3};
const struct unit_type ram = {'R', 90, 2, 500, 1, 4};
const struct unit_type catapult = {'C', 50, 2, 800, 7, 6};
const struct unit_type worker = {'W', 20, 2, 100, 1, 2};
const struct unit_type base = {'B', 200, 0};


int get_total_number_of_units(char file_name[MAX_STRING_SIZE]);
bool load_status (char file_name[MAX_STRING_SIZE], struct status_info *status, struct unit* units, char *curr_player);
bool load_map (char file_name[MAX_STRING_SIZE], struct map_state *map);
bool is_file_empty(char file_name[MAX_STRING_SIZE]);
bool write_first_status_file(struct map_state *map_ptr, char file_name[MAX_STRING_SIZE]);
bool run_player_program(struct status_info *status, char *map_file, char *status_file, char *orders_file, char t_limit, char *player, char limit);
bool game_over (int round, int mzx_rounds, int units_player1, int units_player2, char player1_base, char player2_base);
void add_gold(struct status_info *status);
bool move(struct status_info *status, int id1, int x, int y, int num_of_units, char *player, struct map_state *map);
bool find_bases(struct status_info *status, int *total_num_of_units, const struct unit_type* unittypes[], char player[]);
bool is_base_busy(struct status_info *status, int unit, int id, int *total_num_of_units, const struct unit_type* unittypes[], char player[]);
int get_unit_attack_index_by_notation(char unit_notation, const struct unit_type* unittypes[]);
const struct unit_type* get_type_by_notation(char unit_notation, const struct unit_type* unittypes[]);
bool attack(struct status_info *status, int id1, int id2, int num_of_units, char *player, struct map_state *map, const struct unit_type* unit_types[]);
bool build(struct status_info *status, int id, char unit_to_build, int num_of_units, char *player, const struct unit_type* unittypes[]);
bool is_coordinates_taken(struct status_info *status, int x, int y, int num_of_units, struct unit u);
bool check_map_range(int x, int y, struct map_state *map_ptr);
bool is_mine(int x, int y, struct map_state *map_ptr);
bool is_base(int x, int y, struct map_state *map_ptr);
bool is_obstacle(int x, int y, struct map_state *map_ptr);
bool load_orders (char file_name[MAX_STRING_SIZE], char *player, struct status_info *status, struct map_state *map, int num_of_units, const struct unit_type* unit_types[]);
int get_attack_damage(struct unit *attacker, struct unit *target, const struct unit_type* unittypes[]);
int get_distance(int x1, int x2, int y1, int y2);
void print_winner(char *player);
void rewrite_status_file(char file_name[MAX_STRING_SIZE], struct status_info *status, int num_of_units, char player[]);
char swap_players(char *current_p, char units_p);

#endif