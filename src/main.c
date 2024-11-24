#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "map.h"
#include "loc.h"
#include "moves.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#define MAX_CHILDREN 9
#define MAX_PATH_LENGTH 6

// define x and y axis
#define X 4
#define Y 6

int u_turn_choice = -1;
// Struct of tree
typedef struct t_node {
    char path[MAX_PATH_LENGTH];
    int val;
    struct t_node *children[MAX_CHILDREN];
} t_node;


// Struct of tree root
typedef struct s_tree
{
    t_node *root;
} t_tree;

int get_u_turn_choice() {
    if (u_turn_choice == -1) { // Générer une seule fois si pas encore initialisé
        srand(time(NULL));
        u_turn_choice = rand() % 2;
    }
    return u_turn_choice;
}

void printPath(const char *prefix, const char *string) {
    printf("%s ", prefix);
    if (string != NULL)
        printf("%s", string);
    else 
        printf("(null)");
    printf("\n");
}

char* arrayrandomproba() {
    char *arrayrandomproba = (char*)malloc((MAX_CHILDREN + 1) * sizeof(char));
    if (arrayrandomproba == NULL) {
        return NULL;
    }

    int SIZE = 100;
    int F_10 = 22, F_20 = 15, F_30 = 7;
    int R_10 = 7, TR = 21, TL = 21, TB = 7;

    srand(time(0)); // NOLINT(*-msc51-cpp)

    char arrayloop[SIZE];
    int index = 0;

    for (int i = 0; i < F_10; i++) arrayloop[index++] = 'A';
    for (int i = 0; i < F_20; i++) arrayloop[index++] = 'B';
    for (int i = 0; i < F_30; i++) arrayloop[index++] = 'C';
    for (int i = 0; i < R_10; i++) arrayloop[index++] = 'R';
    for (int i = 0; i < TR; i++) arrayloop[index++] = 'T';
    for (int i = 0; i < TL; i++) arrayloop[index++] = 'L';
    for (int i = 0; i < TB; i++) arrayloop[index++] = 'J';

    for (int i = 0; i < 9; i++) {
        int randomnumber = rand() % SIZE; // NOLINT(*-msc50-cpp)
        arrayrandomproba[i] = arrayloop[randomnumber];

        switch (arrayrandomproba[i]) {
            case 'A': F_10--; break;
            case 'B': F_20--; break;
            case 'C': F_30--; break;
            case 'R': R_10--; break;
            case 'T': TR--; break;
            case 'L': TL--; break;
            case 'J': TB--; break;
        }

        for (int j = randomnumber; j < SIZE - 1; j++) {
            arrayloop[j] = arrayloop[j + 1];
        }
        SIZE--;
    }

    arrayrandomproba[MAX_CHILDREN] = '\0';
    return arrayrandomproba;
}

t_tree createEmptyTree(){
    t_tree temp;
    temp.root = NULL;
    return temp;
}

int out_of_bounds(t_localisation loc) {
    return loc.pos.x < 0 || loc.pos.x >= 6 || loc.pos.y < 0 || loc.pos.y >= 7;
}

int get_cost(t_map map, t_localisation loc) {
    return map.costs[loc.pos.y][loc.pos.x];
}

int is_crevasse(int cost) {
    return cost >= 10000;
}

// Map created with dimensions 7 x 6
int calculate_node(char* t_path, t_map map, t_localisation loc,int* reg) {
    t_localisation phantom_loc = loc;
    int size = strlen(t_path), crevasse = 0;
    int cost;

    for (int i = 0; i < size; i++) {
        int steps = 1;  // Default step count for 'A' and 'R'

        switch (t_path[i]) {
            case 'A':
                steps = 1;
                break;
            case 'B':
                steps = 2;
                break;
            case 'C':
                steps = 3;
                break;
            case 'R':
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)continue;
                phantom_loc = translate(phantom_loc, B_10);
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==REG)*reg = 1;
                if (out_of_bounds(phantom_loc)) return 999999;  // Impossible Path
                cost = get_cost(map, phantom_loc);
                if (is_crevasse(cost)) crevasse = 1;
                continue;  // Skip the rest of this loop iteration
            case 'T':
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)continue;
                phantom_loc.ori = rotate(phantom_loc.ori, T_RIGHT);
                continue;
            case 'L':
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)continue;
                phantom_loc.ori = rotate(phantom_loc.ori, T_LEFT);
                continue;
            case 'J':
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG) {
                    if (get_u_turn_choice() == 0) {
                        phantom_loc.ori = rotate(phantom_loc.ori, T_LEFT);
                        continue;
                    } else {
                        phantom_loc.ori = rotate(phantom_loc.ori, T_RIGHT);
                        continue;
                    }
                }
                phantom_loc.ori = rotate(phantom_loc.ori, U_TURN);
                continue;
            default:
                continue;  // Ignore invalid commands
        }
        if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)steps--;
        for (int step = 0; step < steps; step++) {
            phantom_loc = translate(phantom_loc, F_10);
            if (out_of_bounds(phantom_loc)) return 999999;  // Impossible Path
            cost = get_cost(map, phantom_loc);
            if (is_crevasse(cost)) crevasse = 1;
            if (cost == 0) break;  // Stop early if cost is 0
        }
        if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==REG)*reg = 1;
    }

    if (out_of_bounds(phantom_loc)) return 999999;  // Impossible Path
    if (crevasse) return 10000;  // Found crevasse during traversal

    return get_cost(map, phantom_loc);
}

t_node *createNode(char *t_path, t_map map, t_localisation loc,int max_path_lenght, int* reg) {
    t_node *node = (t_node *)malloc(sizeof(t_node));
    strncpy(node->path, t_path, max_path_lenght - 1);
    node->path[max_path_lenght - 1] = '\0';

    node->val = calculate_node(t_path, map, loc, reg);



    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    // printf("node info: path: %s, val: %d\n", node->path, node->val);
    return node;
}

void generateCombinations(t_node *node, const char *alphabet, int depth, int maxDepth, t_map map, t_localisation loc,int* reg) { // NOLINT(*-no-recursion)
    if (depth == maxDepth) {
        return;
    }

    int len = strlen(alphabet); // NOLINT(*-narrowing-conversions)
    for (int i = 0; i < len; i++) {
        // Create new chain alphabet[i]
        char reducedAlphabet[len];
        strncpy(reducedAlphabet, alphabet, i); // Copy letters before i
        strncpy(reducedAlphabet + i, alphabet + i + 1, len - i - 1); // Copy letters after i
        reducedAlphabet[len - 1] = '\0';

        // Add to current path
        char newPath[MAX_PATH_LENGTH];
        snprintf(newPath, MAX_PATH_LENGTH, "%s%c", node->path, alphabet[i]);

        t_node *child = createNode(newPath, map, loc, MAX_PATH_LENGTH, reg);
        node->children[i] = child;

        generateCombinations(child, reducedAlphabet, depth + 1, maxDepth, map, loc, reg);
    }
}

void findOptimalPath(t_node* node, t_node** optimalNode, int* minCost, char** optimalPath) { // NOLINT(*-no-recursion)
    if (node == NULL) return;
    int len = strlen(node->path); // NOLINT(*-narrowing-conversions)
    if (node->val < *minCost ||
        (node->val == *minCost && (*optimalPath == NULL || len < strlen(*optimalPath))) ) {
        if (len == 5 || node->val == 0) {
            *minCost = node->val;
            *optimalNode = node;
            *optimalPath = node->path;
        }
    }

    for (int i = 0; i < 9; i++) {
        findOptimalPath(node->children[i], optimalNode, minCost, optimalPath);
    }
}

void printTree(t_node *node, int depth) { // NOLINT(*-no-recursion)
    if (node == NULL) return;

    // Indentations
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    // Node information
    printf("Node(Path: %s, Value: %d)\n", node->path, node->val);

    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (node->children[i] != NULL) {
            printTree(node->children[i], depth + 1);
        }
    }
}

void freeTree(t_node *node) { // NOLINT(*-no-recursion)
    if (node == NULL) return;

    for (int i = 0; i < MAX_CHILDREN; i++) {
        freeTree(node->children[i]);
    }
    free(node);
}

int countNodes(t_node *node) { // NOLINT(*-no-recursion)
    if (node == NULL) {
        return 0;
    }

    int count = 1;

    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (node->children[i] != NULL) {
            count += countNodes(node->children[i]);
        }
    }

    return count;
}

void displayNode(t_node *node, int depth) { // NOLINT(*-no-recursion)
    if (node == NULL) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("Value(%d): %d\n", depth, node->val);

    for (int i = 0; i < 9; i++) {
        displayNode(node->children[i], depth + 1);
    }
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

t_localisation live_map_preview(char* t_path, t_map map, t_localisation loc, int minCost, int* reg) {
    t_localisation phantom_loc = loc; // Initial position
    int size = strlen(t_path); // NOLINT(*-narrowing-conversions)

    printf(GREEN "\n=== Initial Map Preview ===\n" RESET);
    displayMap(map, loc);
    printf("\nRover starting at position: (%d, %d), facing: %d\n", loc.pos.x, loc.pos.y, loc.ori);
    printf(YELLOW "Optimal Path to Follow: %s\n" RESET, t_path);
    printf(RED "Estimated Minimum Cost: %d\n" RESET, minCost);
    printf("\n");

    for (int i = 0; i < size; i++) {
        int steps;

        switch(t_path[i]) {
            case 'A': // Forward 10
                steps = 1;
                break;
            case 'B': // Forward 20 (10+10)
                steps = 2;
                break;
            case 'C': // Forward 30 (10+10+10)
                steps = 3;
                break;
            case 'R': // Backward 10
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)continue;
                phantom_loc = translate(phantom_loc, B_10);
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==REG)*reg = 1;
                clear_screen();
                displayMap(map, phantom_loc);
                printf(YELLOW "\nRover moved backward.\n" RESET);
                usleep(1000000);
                continue;

            case 'T': // Right rotation
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)continue;
                phantom_loc.ori = rotate(phantom_loc.ori, T_RIGHT);
                clear_screen();
                displayMap(map, phantom_loc);
                printf(CYAN "\nRover turned right.\n" RESET);
                usleep(1000000);
                continue;

            case 'L': // Left Rotation
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)continue;
                phantom_loc.ori = rotate(phantom_loc.ori, T_LEFT);
                clear_screen();
                displayMap(map, phantom_loc);
                printf(CYAN "\nRover turned left.\n" RESET);
                usleep(1000000);
                continue;

            case 'J': // U-turn
                if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG) {
                    if (get_u_turn_choice() == 0) {
                        phantom_loc.ori = rotate(phantom_loc.ori, T_LEFT);
                        clear_screen();
                        displayMap(map, phantom_loc);
                        printf(CYAN "\nRover turned left.\n" RESET);
                        usleep(1000000);
                        continue;
                    } else {
                        phantom_loc.ori = rotate(phantom_loc.ori, T_RIGHT);
                        clear_screen();
                        displayMap(map, phantom_loc);
                        printf(CYAN "\nRover turned right.\n" RESET);
                        usleep(1000000);
                        continue;
                    }
                }
                phantom_loc.ori = rotate(phantom_loc.ori, U_TURN);
                clear_screen();
                displayMap(map, phantom_loc);
                printf(CYAN "\nRover made a U-turn.\n" RESET);
                usleep(1000000);
                continue;

            default:
                continue;
        }
        if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==ERG)steps--;
        for (int step = 0; step < steps; step++) {
            phantom_loc = translate(phantom_loc, F_10);
            clear_screen();
            displayMap(map, phantom_loc);
            printf(YELLOW "\nRover moved forward (%d/%d steps).\n" RESET, step + 1, steps);
            usleep(1000000);
            if (map.soils[phantom_loc.pos.y][phantom_loc.pos.x] == BASE_STATION)break;
        }
        if(map.soils[phantom_loc.pos.y][phantom_loc.pos.x]==REG)*reg = 1;
    }

    // final result
    clear_screen();
    displayMap(map, phantom_loc);
    printf(GREEN "\n=== Final State ===\n" RESET);
    printf("Optimal path: " MAGENTA "%s\n" RESET, t_path);
    printf("Cost: " CYAN "%d\n" RESET, minCost);
    printf("( ˶ˆᗜˆ˵ )\n");
    return phantom_loc;
}

void needinfo() {
    t_map map = createMapFromFile("./maps/example1.map");

    int Xdep = X;
    int Ydep = Y;
    int reg = 0;
    t_orientation ori = NORTH;

    t_localisation loc = loc_init(Xdep, Ydep, ori);

    clock_t start, end;
    clock_t start1,start2,end1,end2;
    double cpu_time_used1, cpu_time_used2,cpu_time_used3;
    start = clock();

    char* alphabet = arrayrandomproba();
    int maxDepth = 5;

    t_tree tree = createEmptyTree();
    t_node *root = createNode("", map, loc, MAX_PATH_LENGTH, &reg);
    tree.root = root;

    start1 = clock();
    generateCombinations(root, alphabet, 0, maxDepth, map, loc, &reg);
    end1 = clock();

    t_node *optimalNode = NULL;
    int minCost = 9000;
    char* optimalPath = NULL;
    start2 = clock();
    findOptimalPath(root, &optimalNode, &minCost, &optimalPath);
    end2 = clock();

    calculate_node(optimalPath, map, loc, &reg);

    end = clock();
    cpu_time_used1 = ((double) (end - start)) / CLOCKS_PER_SEC;
    cpu_time_used2 = ((double) (end1 - start1)) / CLOCKS_PER_SEC;
    cpu_time_used3 = ((double) (end2 - start2)) / CLOCKS_PER_SEC;


    printf("\n      __...--~~~~~-._   _.-~~~~~--...__\n");
    printf("    //               `V'               \\ \n");
    printf("   //                 |                 \\ \n");
    printf("  //__...--~~~~~~-._  |  _.-~~~~~~--...__\\ \n");
    printf(" //__.....----~~~~._\\ | /_.~~~~----.....__\\\n");
    printf("====================\\\\|//====================\n");
    printf("                     `---`\n");
    printf("///////////////// INFORMATIONS ///////////////////");

    printf("\n");

    printf("Time taken to execute the generateCombinations function : %f\n", cpu_time_used2);
    printf("Time taken to execute the findOptimalPath function : %f\n", cpu_time_used3);
    printf("Time taken to execute all the project: %f\n\n", cpu_time_used1);
    printf("Path combinaison: %s\n", alphabet);
    printf("Number of nodes in the tree: %d\n", countNodes(root));
    printf("Optimal path: %s\n", optimalPath);
    printf("Cost: %d\n", minCost);
    printf("Rover reached the base: %s\n\n", minCost == 0 ? "Yes" : "No");
    free(alphabet);
    usleep(2000000);
}

void gamble() {
    srand(time(NULL));

    const char *roulette_options[15] = {
        "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "10", "11", "12", "13", "14"
    };

    int user_guess;
    printf("Gamble the cost (between 0 and 14): ");
    scanf("%d", &user_guess);

    if (user_guess < 0 || user_guess > 14) {
        printf("Error: Invalid guess. Please try again.\n");
        return;
    }

    int user_money;
    printf("Enter the amount of money to gamble: ");
    scanf("%d", &user_money);

    if (user_money <= 0) {
        printf("Error: Invalid amount. Please enter a positive value.\n");
        return;
    }

    int spins = 20;
    int delay = 100000;
    printf("\nLUNCHING ROULETTA...\n");


    t_map map = createMapFromFile("./maps/example1.map");
    int Xdep = X;
    int Ydep = Y;
    int reg=0;
    t_orientation ori = NORTH;

    t_localisation loc = loc_init(Xdep, Ydep, ori);
    int maxDepth = 5; // max Depth for permutations

    t_tree tree = createEmptyTree();
    t_node *root = createNode("", map, loc, MAX_PATH_LENGTH, &reg);
    tree.root = root;

    // Search Better path
    char final_path[50];
    final_path[0]=' ';
    char* alphabet;
    t_node *optimalNode = NULL;
    int minCost = 9000;
    char* optimalPath = NULL;

    alphabet = arrayrandomproba();
    generateCombinations(root, alphabet, 0, maxDepth, map, loc, &reg);
    findOptimalPath(root, &optimalNode, &minCost, &optimalPath);

    for (int i = 0; i < spins; i++) {
        clear_screen();

        int random_index = rand() % 15;
        printf("\n\t========= ROULLETA =========\n");
        printf("\t           %s\n", roulette_options[random_index]);
        printf("\t============================\n");

        usleep(delay);
        delay += 20000;
    }

    clear_screen();
    printf("\n\t===== Final Result =====\n");
    printf("\t      %s\n", roulette_options[minCost]);
    printf("\t========================\n");

    if (user_guess == minCost) {
        printf("\nVictory 🎉🐒! You guessed it right.\n");
        int payout = 2; // Payout ratio: 2:1
        int money_won = user_money * payout;
        printf("You won $%d!\n\n", money_won);
    } else {
        printf("\nDefeat 🙈🐵! The correct answer is %d.\n", minCost);
        printf("You lost $%d.\n\n", user_money);
    }
}

void loadingMenu() {
    const char *rover = 
        "[\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"]\n"
        "[ \\--------MARS ROVER PROJECT - 2024-----~~~~~~~~~~~~~~ ]\n"
        "[ |                                          {EFREI ///} ]\n"
        "[ |      Rayan Hector Ambroise                  { /*\\/} ]\n"
        "[ |                                          {  /* *\\}  ]\n"
        "[ |                                           ~~~~~~~|   ]\n"
        "[ |              __                       _          |   ]\n"
        "[ |             /\\ `\\_                   /\\`\\_      |]\n"
        "[ |            /  ~   \\      .          /  ~  \\      | ]\n"
        "[ |___________/________\\_____|_________/_______\\_____| ]\n"
        "[ |   .^^____  ^       ______|_^.^  ___ ^ .  _ .^^  .|   ]\n"
        "[ |.^. _/   _\\_^  Q]-,  | __ |_  ^ |   \\ ^^ / \\  ^. ^|]\n"
        "[ | ^ |   '   \\   \\_|/__\\_|_ ^ \\____\\.^ \\__\\ ^ ^.|]\n"
        "[ |^.^\\____\\____\\^.^  (o):(o):(o)::::::::::::::::::::|]\n"
        "[ /---------------------------------------------------\\ ]\n"
        "  \"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\n";

    printf("%s", rover);
    usleep(5000000);
    clear_screen();
}

void livePreview() {
    t_map map = createMapFromFile("./maps/example1.map");

    //printf("Map created with dimensions %d x %d\n", map.y_max, map.x_max);
    for (int i = 0; i < map.y_max; i++)
    {
        for (int j = 0; j < map.x_max; j++)
        {
            //printf("%d ", map.soils[i][j]);
        }
        printf("\n");
    }
    // printf the costs, aligned left 5 digits
    for (int i = 0; i < map.y_max; i++)
    {
        for (int j = 0; j < map.x_max; j++)
        {
            //printf("%-5d ", map.costs[i][j]);
        }
        printf("\n");
    }



    int Xdep = X;
    int Ydep = Y;
    int reg = 0;
    t_orientation ori = NORTH;

    t_localisation loc = loc_init(Xdep, Ydep, ori);

    displayMap(map, loc);
    printf("Rover starting at position: (%d, %d), facing: %d\n", loc.pos.x, loc.pos.y, loc.ori);
    usleep(3000000);

    clock_t start, end;
    double cpu_time_used;
    start = clock();
    int maxDepth = 5; // max Depth for permutations

    t_tree tree = createEmptyTree();
    t_node *root = createNode("", map, loc, MAX_PATH_LENGTH, &reg);
    tree.root = root;
    
    // printf("Nb node: %d\n", countNodes(root));

    // Search Better path
    char final_path[50];
    final_path[0]=' ';
    char* alphabet;
    t_node *optimalNode = NULL;
    int minCost = 9000;
    char* optimalPath = NULL;
    while(minCost != 0) {
        alphabet = arrayrandomproba();
        generateCombinations(root, alphabet, 0, maxDepth-reg, map, loc, &reg);
        reg = 0;
        findOptimalPath(root, &optimalNode, &minCost, &optimalPath);

        printPath("Generated array: ", alphabet);
        printf("\n\n### Final Best Path Rover Find ###\n");
        if (optimalPath != NULL) {
            printf("Optimal path: %s\n", optimalPath);
            printf("Cost: %d\n", minCost);
            printf("( ˶ˆᗜˆ˵ )\n");
            calculate_node(optimalPath, map, loc, &reg);
            strcat(final_path, optimalPath);
        } else
            printf("Sadly no path find...\n૮(˶ㅠ︿ㅠ)ა\n");


        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Time taken : %f\n", cpu_time_used);

        loc = live_map_preview(optimalPath, map, loc, minCost, &reg);
        if(minCost!= 0){
            printf("Draft finished without reaching the base");
            usleep(2000000);
        }
        free(alphabet);

    }
    printf("Final path :%s\n\n",final_path);
}

void mainMenu() {
    int option;
    do {
        printf("Main Menu\n");
        printf("1. Live Preview\n");
        printf("2. Gamble\n");
        printf("3. More information\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                livePreview();
                break;
            case 2:
                gamble();
                break;
            case 3:
                needinfo();
                break;
            case 4:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    } while (option != 4);
}

int main() {
    loadingMenu();
    clear_screen();
    mainMenu();
    return 0;
}
