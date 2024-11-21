#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "map.h"
#include "loc.h"
#include "moves.h"

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

    srand(time(0));

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
        int randomnumber = rand() % SIZE;
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
    t_orientation ori = NORTH;
    t_localisation phantomloc = loc;
    int size = strlen(t_path);

    for (int i = 0; i < size; i++) {
        switch(t_path[i]) {
            case 'A': phantomloc = translate(phantomloc, F_10); break;
            case 'B':
                for(int wrizz=0;wrizz<2;wrizz++)
                    phantomloc = translate(phantomloc, F_10);
                break;
            case 'C':
                for(int wrizz=0;wrizz<3;wrizz++)
                    phantomloc = translate(phantomloc, F_10);
                break;
            case 'R': phantomloc = translate(phantomloc, B_10); break;
            case 'T': phantomloc.ori = rotate(phantomloc.ori, T_RIGHT); break;
            case 'L': phantomloc.ori = rotate(phantomloc.ori, T_LEFT); break;
            case 'J': phantomloc.ori = rotate(phantomloc.ori, U_TURN); break;
            default: break;
        }
    }
    if (phantomloc.pos.x < 0 || phantomloc.pos.x >= 7 || phantomloc.pos.y < 0 || phantomloc.pos.y >= 6) {
        // printf("Erreur : Rover out of range\n");
        return 999999;  // Imposible Path
    }
    int Cost = map.costs[phantomloc.pos.y][phantomloc.pos.x];

    // printf("Position: (%d, %d), Orientation %d cost: %d\n", phantomloc.pos.x, phantomloc.pos.y, phantomloc.ori, totalCost);
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

void generateCombinations(t_node *node, const char *alphabet, int depth, int maxDepth, t_map map, t_localisation loc) {
    if (depth == maxDepth) {
        //printf("Chemin final : %s\n", node->path);
        return;
    }

    int len = strlen(alphabet);
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


// Reste plus qu'a fix cette fonction car deja j'ai pas compris comment ca peut marcher et j'arrive pas a return tout le path dcp force a nous
void findOptimalPath(t_node* node, t_node** optimalNode, int* minCost, char** optimalPath) {
    if (node == NULL) return;

    if (node->val < *minCost && (strlen(node->path) == 5 || node->val == 0)) {
        *minCost = node->val;
        *optimalNode = node;
        *optimalPath = node->path;
    }

    for (int i = 0; i < 9; i++) {
        findOptimalPath(node->children[i], optimalNode, minCost, optimalPath);
    }
}





void printTree(t_node *node, int depth) {
    if (node == NULL) return;

    // Indentations
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    // Node informations
    printf("Node(Path: %s, Value: %d)\n", node->path, node->val);

    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (node->children[i] != NULL) {
            printTree(node->children[i], depth + 1);
        }
    }
}

void freeTree(t_node *node) {
    if (node == NULL) return;

    for (int i = 0; i < MAX_CHILDREN; i++) {
        freeTree(node->children[i]);
    }
    free(node);
}

int countNodes(t_node *node) {
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










void displayNode(t_node *node, int depth) {
    if (node == NULL) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("Value(%d): %d\n", depth, node->val);

    for (int i = 0; i < 9; i++) {
        displayNode(node->children[i], depth + 1);
    }
}

void displayTree(t_tree *tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("L'arbre est vide.\n");
        return;
    }
    printf("Affichage de l'arbre:\n");
    displayNode(tree->root, 0);
}



void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// show the map in real time
void live_map_preview(char* t_path, t_map map, t_localisation loc, int minCost) {
    t_orientation ori = NORTH; // Orientation initiale
    t_localisation phantomloc = loc; // Position initiale
    int size = strlen(t_path);

    // Afficher un aperçu de la carte avant la boucle
    printf(GREEN "\n=== Initial Map Preview ===\n" RESET);
    displayMap(map, loc);
    printf("\nRover starting at position: (%d, %d), facing: %d\n", loc.pos.x, loc.pos.y, loc.ori);
    printf(YELLOW "Optimal Path to Follow: %s\n" RESET, t_path);
    printf(RED "Estimated Minimum Cost: %d\n" RESET, minCost);
    printf("\n");

    // Boucle pour afficher les étapes
    for (int i = 0; i < size; i++) {
        switch(t_path[i]) {
            case 'A': phantomloc = translate(phantomloc, F_10); break;
            case 'B':
                for(int wrizz=0;wrizz<2;wrizz++)
                    phantomloc = translate(phantomloc, F_10);
                break;
            case 'C':
                for(int wrizz=0;wrizz<3;wrizz++)
                    phantomloc = translate(phantomloc, F_10);
                break;
            case 'R': phantomloc = translate(phantomloc, B_10); break;
            case 'T': phantomloc.ori = rotate(phantomloc.ori, T_RIGHT); break;
            case 'L': phantomloc.ori = rotate(phantomloc.ori, T_LEFT); break;
            case 'J': phantomloc.ori = rotate(phantomloc.ori, U_TURN); break;
            default: break;
        }
        usleep(1000000); // Pause d'une seconde entre chaque étape
        clear_screen(); // Nettoyer l'écran
        printf(BLUE "\n=== Step %d ===\n" RESET, i + 1);
        displayMap(map, phantomloc);
        printf("\n### Final Best Path Rover Find ###\n");
        printf("Optimal path: " MAGENTA "%s\n" RESET, t_path);
        printf("Cost: " CYAN "%d\n" RESET, minCost);
        printf("( ˶ˆᗜˆ˵ )\n");
    }
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



    int Xdep = 4;
    int Ydep = 6;
    t_orientation ori = NORTH;

    t_localisation loc = loc_init(Xdep, Ydep, ori);

    displayMap(map, loc);
    printf("x axis: %d, y axis: %d\n", loc.pos.x, loc.pos.y);

    clock_t start, end;
    double cpu_time_used;
    start = clock();
    
    char* alphabet = arrayrandomproba();
    int maxDepth = 5; // Profondeur maximale pour les combinaisons

    t_tree tree = createEmptyTree();
    t_node *root = createNode("", map, loc);
    tree.root = root;



    generateCombinations(root, alphabet, 0, maxDepth, map, loc);
    
    //printTree(root, 0);
    
    // printf("Nb node: %d\n", countNodes(root));




    // Libération mémoire (à implémenter si nécessaire)


    // Search Better path
    t_node *optimalNode = NULL;
    int minCost = 9000;
    char* optimalPath = NULL;

    findOptimalPath(root, &optimalNode, &minCost, &optimalPath);

    printPath("Generated array: ", alphabet);
    printf("\n\n### Final Best Path Rover Find ###\n");
    if (optimalPath != NULL) {
        printf("Optimal path: %s\n", optimalPath);
        printf("Cost: %d\n", minCost);
        printf("( ˶ˆᗜˆ˵ )\n");
        calculate_node(optimalPath, map, loc);
    } else
        printf("Safly no path find...\n૮(˶ㅠ︿ㅠ)ა\n");

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken : %f\n", cpu_time_used);


    live_map_preview(optimalPath, map, loc, minCost);

    free(alphabet);
    return 0;
}