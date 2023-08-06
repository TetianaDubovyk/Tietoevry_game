# Tietoevry strategy game
A turn based strategy game, written in C.
## Description
There are two applications in this project: mediator and player.<br>
mediator.c contains the code to run the game<br>
player.c contains the logic for making optimal moves

Mediator run the Player program each round, as well as load data from files, check the correctness of data prepared by the Player program, update the game state, write a new "status.txt" file for the pPlayer program, determine the winner of the game.<br>
The pPlayer program plan actions for its units based on the loaded information from the "status.txt" and "map.txt", and write orders to be done into the "orders.txt" file.

Mediator runs from the command line as follows:
```
.\mediator map.txt status.txt orders.txt timelimit(optional)
```
Player runs as follows:
```
.\player map.txt status.txt orders.txt timelimit(optional)
```

* "map.txt" contains a map for the game (max size of map is predefined in the header files for both programs)<br>
* "status.txt" contains status and location of all available units<br>
* "orders.txt" contains orders for individual units, written by player program<br>
* timelimit - maximum time specified in seconds for the player's move<br>

The maximum duration of the game is 2000 turns, 1000 turns for each of two players. 
Each player has 8 different types of units, including a base, which can produce new units. On the map there is obstacles, which can be jumped over and mines, where units of the type 'worker' can mine gold.
The goal is to destroy the opponent's base. If the max duration of the game is reached and neither of the bases are destroyed, the player with the biggest number of units will be the winner, in the case of an equal number of units, game ends with a tie.

The full version of the rules for the game is in TietoEvry_task.docx file (in Polish).

## About the algorithm for the Player
The Player program is a simple AI agent based on the rule-based approach. Agent defines the tasks and assignes them to available units. 
The top-level tasks are:
1. Player's base protection. Agent calculates relative forces balance near the base considering all ally's and opponent's units located at most maximun possible attack radius.
Opponent's units intended to cause damage specifically for the base (catapult, ram) cause bigger threat that e.g. pikeman. The player's units should counterbalance the threat and agent selects 
the most appropriate ones, considering their relative strenght, distance from base and damage they cause for typical base-assault units.
All heuristic calculations based on the unit relative strenght, which is the sum of attacking characteristics divided by max possible value of each one. The relative strenght for each units is a product of that sum and remaining health ratio.
2. Capture of a mine, if all the mines are busy with enemy's workers.
3. Attacking opponent's base. All free player's units ordered to move near the enemy's base and attack it and it's protectors.
4. Sending free workers to mines.
5. Order to build new unit. The unit type to build is determined by simple heuristic which is a value function, considering current amount of gold, unit's relative strenght, cost and time to build, depending on the current task of highest priority (e.g. build knight to protect base or build worker if gold is running out).

These top-level tasks are decomposed into several subtasks, like move each unit to specific position, attack required targets, e.t.c.


