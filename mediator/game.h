#ifndef GAME_HEAD_H
#define GAME_HEAD_H

#define MAX_STRING_SIZE 40
#define MAX_SIZE_OF_MAP 100
#define NUM_OF_TYPES 8
#define UNREACHABLE_DISTANCE -1
#define MAX_MINES 50
#define MAX_OBSTACLES 350
#define P "Player #1"

#include <linux/limits.h>


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
    struct coordinates bases[2];
    struct coordinates mines[MAX_MINES];
    struct coordinates obstacles[MAX_OBSTACLES];
};


extern const struct unit_type knight;
extern const struct unit_type swordsman;
extern const struct unit_type archer;
extern const struct unit_type pikeman;
extern const struct unit_type ram;
extern const struct unit_type catapult;
extern const struct unit_type worker;
extern const struct unit_type base;


int get_total_number_of_units(char file_name[PATH_MAX]);
bool validate_status_file(char file_name[PATH_MAX]);
bool load_status (char file_name[PATH_MAX], struct status_info *status, struct unit* units, char *curr_player);
bool load_map (char file_name[PATH_MAX], struct map_state *map);
bool is_file_empty(char file_name[PATH_MAX]);
bool validate_status_base_line_chars(char player, char unit_type, char unit_type_being_built);
bool validate_player_char(char p);
bool validate_unit_type(char type);
bool write_first_status_file(struct map_state *map_ptr, char file_name[PATH_MAX]);
bool run_player_program(struct status_info *status, char *map_file, char *status_file, char *orders_file, char t_limit, char *player, char limit);
bool game_over (int round, int mzx_rounds, struct status_info *status, struct map_state *map);
void add_gold(struct status_info *status);
bool move(struct status_info *status, int id1, int x, int y, int num_of_units, char *player, struct map_state *map);
bool find_bases(struct status_info *status, int *total_num_of_units, const struct unit_type* unittypes[], char player[]);
bool is_base_busy(struct status_info *status, int unit, int id, int total_num_of_units, const struct unit_type* unittypes[], char player[]);
int get_unit_attack_index_by_notation(char unit_notation, const struct unit_type* unittypes[]);
const struct unit_type* get_type_by_notation(char unit_notation, const struct unit_type* unittypes[]);
bool attack(struct status_info *status, int id1, int id2, int num_of_units, char *player, struct map_state *map, const struct unit_type* unittypes[]);
bool build(struct status_info *status, int id, char unit_to_build, int num_of_units, char *player, const struct unit_type* unittypes[]);
bool is_coordinates_taken(struct status_info *status, int x, int y, int num_of_units, struct unit u);
bool check_map_range(int x, int y, struct map_state *map_ptr);
bool is_mine(int x, int y, struct map_state *map_ptr);
bool is_base(int x, int y, struct map_state *map_ptr);
bool is_obstacle(int x, int y, struct map_state *map_ptr);
bool load_orders (char file_name[PATH_MAX], char *player, struct status_info *status, struct map_state *map, int num_of_units, const struct unit_type* unittypes[]);
int get_attack_damage(struct unit *attacker, struct unit *target, const struct unit_type* unittypes[]);
int get_distance(int x1, int x2, int y1, int y2);
void print_winner(char *player);
void rewrite_status_file(char file_name[PATH_MAX], struct status_info *status, int num_of_units, char player[]);
char swap_players(char *current_p, char units_p);
void free_memory (struct status_info *status, struct map_state *map, struct unit *unit);

#endif
