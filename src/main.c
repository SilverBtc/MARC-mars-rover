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

t_tree createEmptyTree(){
    t_tree temp;
    temp.root = NULL;
    return temp;
}




// Map created with dimensions 7 x 6
int calculate_node(char* t_path, t_map map, t_localisation loc) {
    t_localisation phamtom_loc = loc;
    int size = strlen(t_path), crevasse = 0; // NOLINT(*-narrowing-conversions)
    int Cost;

    for (int i = 0; i < size; i++) {
        switch(t_path[i]) {
            case 'A': phamtom_loc = translate(phamtom_loc, F_10);
                if (phamtom_loc.pos.x < 0 || phamtom_loc.pos.x >= 6 || phamtom_loc.pos.y < 0 || phamtom_loc.pos.y >= 7) {
                    // printf("Error : Rover out of range\n");
                    return 999999;  // Impossible Path
                }
                Cost = map.costs[phamtom_loc.pos.y][phamtom_loc.pos.x];
                if (Cost >= 10000)crevasse = 1;
            break;
            case 'B':
                for(int step=0;step<2;step++) {
                    phamtom_loc = translate(phamtom_loc, F_10);
                    if (phamtom_loc.pos.x < 0 || phamtom_loc.pos.x >= 6 || phamtom_loc.pos.y < 0 || phamtom_loc.pos.y >= 7) {
                        // printf("Error : Rover out of range\n");
                        return 999999;  // Impossible Path
                    }
                    Cost = map.costs[phamtom_loc.pos.y][phamtom_loc.pos.x];
                    if (Cost >= 10000)crevasse = 1;
                    if (Cost == 0)break;
                }
                break;
            case 'C':
                for(int step=0;step<3;step++) {
                    phamtom_loc = translate(phamtom_loc, F_10);
                    if (phamtom_loc.pos.x < 0 || phamtom_loc.pos.x >= 6 || phamtom_loc.pos.y < 0 || phamtom_loc.pos.y >= 7) {
                        // printf("Error : Rover out of range\n");
                        return 999999;  // Impossible Path
                    }
                    Cost = map.costs[phamtom_loc.pos.y][phamtom_loc.pos.x];
                    if (Cost >= 10000)crevasse = 1;
                    if (Cost == 0)break;
                }
                break;
            case 'R': phamtom_loc = translate(phamtom_loc, B_10);
                if (phamtom_loc.pos.x < 0 || phamtom_loc.pos.x >= 6 || phamtom_loc.pos.y < 0 || phamtom_loc.pos.y >= 7) {
                    // printf("Error : Rover out of range\n");
                    return 999999;  // Impossible Path
                }
                Cost = map.costs[phamtom_loc.pos.y][phamtom_loc.pos.x];
                if (Cost >= 10000)crevasse = 1;
            break;
            case 'T': phamtom_loc.ori = rotate(phamtom_loc.ori, T_RIGHT); break;
            case 'L': phamtom_loc.ori = rotate(phamtom_loc.ori, T_LEFT); break;
            case 'J': phamtom_loc.ori = rotate(phamtom_loc.ori, U_TURN); break;
            default: break;
        }
    }
    if (phamtom_loc.pos.x < 0 || phamtom_loc.pos.x >= 6 || phamtom_loc.pos.y < 0 || phamtom_loc.pos.y >= 7) {
        // printf("Error : Rover out of range\n");
        return 999999;  // Impossible Path
    }
    if(crevasse)return 10000; //found crevasse during forward == impossible

        Cost = map.costs[phamtom_loc.pos.y][phamtom_loc.pos.x];
    //printf("Position: (%d, %d), Orientation %d cost: %d\n", phamtom_loc.pos.x, phamtom_loc.pos.y, phamtom_loc.ori, Cost);

    return Cost;
}


t_node *createNode(const char *t_path, t_map map, t_localisation loc) {
    t_node *node = (t_node *)malloc(sizeof(t_node));

    strncpy(node->path, t_path, MAX_PATH_LENGTH - 1);
    node->path[MAX_PATH_LENGTH - 1] = '\0';

    node->val = calculate_node(t_path, map, loc);



    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    // printf("node info: path: %s, val: %d\n", node->path, node->val);
    return node;
}

void generateCombinations(t_node *node, const char *alphabet, int depth, int maxDepth, t_map map, t_localisation loc) { // NOLINT(*-no-recursion)
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

        t_node *child = createNode(newPath, map, loc);
        node->children[i] = child;

        generateCombinations(child, reducedAlphabet, depth + 1, maxDepth, map, loc);
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

t_localisation live_map_preview(char* t_path, t_map map, t_localisation loc, int minCost) {
    t_localisation phamtom_loc = loc; // Initial position
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
                phamtom_loc = translate(phamtom_loc, B_10);
                clear_screen();
                displayMap(map, phamtom_loc);
                printf(YELLOW "\nRover moved backward.\n" RESET);
                usleep(1000000);
                continue;

            case 'T': // Right rotation
                phamtom_loc.ori = rotate(phamtom_loc.ori, T_RIGHT);
                clear_screen();
                displayMap(map, phamtom_loc);
                printf(CYAN "\nRover turned right.\n" RESET);
                usleep(1000000);
                continue;

            case 'L': // Left Rotation
                phamtom_loc.ori = rotate(phamtom_loc.ori, T_LEFT);
                clear_screen();
                displayMap(map, phamtom_loc);
                printf(CYAN "\nRover turned left.\n" RESET);
                usleep(1000000);
                continue;

            case 'J': // U-turn
                phamtom_loc.ori = rotate(phamtom_loc.ori, U_TURN);
                clear_screen();
                displayMap(map, phamtom_loc);
                printf(CYAN "\nRover made a U-turn.\n" RESET);
                usleep(1000000);
                continue;

            default:
                continue;
        }

        for (int step = 0; step < steps; step++) {
            phamtom_loc = translate(phamtom_loc, F_10);
            clear_screen();
            displayMap(map, phamtom_loc);
            printf(YELLOW "\nRover moved forward (%d/%d steps).\n" RESET, step + 1, steps);
            usleep(1000000);
            if (map.soils[phamtom_loc.pos.y][phamtom_loc.pos.x] == 0)break;
        }
    }

    // final result
    clear_screen();
    displayMap(map, phamtom_loc);
    printf(GREEN "\n=== Final State ===\n" RESET);
    printf("Optimal path: " MAGENTA "%s\n" RESET, t_path);
    printf("Cost: " CYAN "%d\n" RESET, minCost);
    printf("( ˶ˆᗜˆ˵ )\n");
    return phamtom_loc;
}






int main() {
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



    int Xdep = 2;
    int Ydep = 4;
    t_orientation ori = NORTH;

    t_localisation loc = loc_init(Xdep, Ydep, ori);

    displayMap(map, loc);
    printf("x axis: %d, y axis: %d\n", loc.pos.x, loc.pos.y);
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    int maxDepth = 5; // max Depth for permutations

    t_tree tree = createEmptyTree();
    t_node *root = createNode("", map, loc);
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
        generateCombinations(root, alphabet, 0, maxDepth, map, loc);
        findOptimalPath(root, &optimalNode, &minCost, &optimalPath);

        printPath("Generated array: ", alphabet);
        printf("\n\n### Final Best Path Rover Find ###\n");
        if (optimalPath != NULL) {
            printf("Optimal path: %s\n", optimalPath);
            printf("Cost: %d\n", minCost);
            printf("( ˶ˆᗜˆ˵ )\n");
            calculate_node(optimalPath, map, loc);
            strcat(final_path, optimalPath);
        } else
            printf("Sadly no path find...\n૮(˶ㅠ︿ㅠ)ა\n");


        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Time taken : %f\n", cpu_time_used);

        loc = live_map_preview(optimalPath, map, loc, minCost);
        if(minCost!= 0){
            printf("Draft finished without reaching the base");
            usleep(2000000);
        }
        free(alphabet);

    }
    printf("Final path :%s\n",final_path);
    return 0;
}
#pragma clang diagnostic pop