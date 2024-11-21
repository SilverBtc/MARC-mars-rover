#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
int calculate_node(char* t_path, t_map map) {
    t_orientation ori = NORTH;
    t_localisation phantomloc = loc_init(3, 3, ori);
    int nodevalue = 9;
    int size = strlen(t_path);
    
    for (int i = 0; i < size; i++) {
        switch(t_path[i]) {
            case 'A': phantomloc = translate(phantomloc, F_10); break;
            case 'B': phantomloc = translate(phantomloc, F_20); break;
            case 'C': phantomloc = translate(phantomloc, F_30); break;
            case 'R': phantomloc = translate(phantomloc, B_10); break;
            case 'T': phantomloc.ori = rotate(phantomloc.ori, T_RIGHT); break;
            case 'L': phantomloc.ori = rotate(phantomloc.ori, T_LEFT); break;
            case 'J': phantomloc.ori = rotate(phantomloc.ori, U_TURN); break;
            default: break;
        }
    }
    if (phantomloc.pos.x < 0 || phantomloc.pos.x > 7 || phantomloc.pos.y < 0 || phantomloc.pos.y > 6) {
        printf("Erreur : Rover out of range\n");
        return 999999;  // Imposible Path
    }

    int cost = map.costs[phantomloc.pos.y][phantomloc.pos.x];
    printf("Position: (%d, %d), Orientation %d\n", phantomloc.pos.x, phantomloc.pos.y, phantomloc.ori);
    return cost;
}


t_node *createNode(const char *t_path, t_map map) {
    t_node *node = (t_node *)malloc(sizeof(t_node));

    strncpy(node->path, t_path, MAX_PATH_LENGTH - 1);
    node->path[MAX_PATH_LENGTH - 1] = '\0';



    int cost = calculate_node(t_path, map);
    node->val = cost; // Calculate cost



    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }


    printf("node info: path: %s, val: %d\n", node->path, node->val);
    return node;
}

void generateCombinations(t_node *node, const char *alphabet, int depth, int maxDepth, t_map map) {
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

        t_node *child = createNode(newPath, map);
        node->children[i] = child;

        generateCombinations(child, reducedAlphabet, depth + 1, maxDepth, map);
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








int main() {
    t_map map = createMapFromFile("./maps/example1.map");
    printf("Map created with dimensions %d x %d\n", map.y_max, map.x_max);
    for (int i = 0; i < map.y_max; i++)
    {
        for (int j = 0; j < map.x_max; j++)
        {
            printf("%d ", map.soils[i][j]);
        }
        printf("\n");
    }
    // printf the costs, aligned left 5 digits
    for (int i = 0; i < map.y_max; i++)
    {
        for (int j = 0; j < map.x_max; j++)
        {
            printf("%-5d ", map.costs[i][j]);
        }
        printf("\n");
    }
    displayMap(map);

    int Xdep = 3;
    int Ydep = 3;
    t_orientation ori = NORTH;

    t_localisation loc = loc_init(Xdep, Ydep, ori);

    printf("x axis: %d, y axis: %d\n", loc.pos.x, loc.pos.y);


    char* alphabet = arrayrandomproba();
    
    // if (alphabet != NULL) {
    //     printf("Generated array: ");
    //     for (int i = 0; i < 9; i++) {
    //         printf("%c", alphabet[i]);
    //     }
    //     printf("\n");
    //     //free(result);
    // }
    printPath("Generated array: ", alphabet);

    // createTree(result, loc, map);

    clock_t start, end;
    double cpu_time_used;
    start = clock();
    

    
    
    // const char *alphabet = "ABCDEFGHI"; // Alphabet de base
    int maxDepth = 5; // Profondeur maximale pour les combinaisons

    t_tree tree = createEmptyTree();
    t_node *root = createNode("", map);
    tree.root = root;



    generateCombinations(root, alphabet, 0, maxDepth, map);
    
    //printTree(root, 0);
    
    printf("Nb node: %d\n", countNodes(root));




    // Libération mémoire (à implémenter si nécessaire)
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken : %f\n", cpu_time_used);


    // printPath("Generated array: ", alphabet);
    // printf("Map created with dimensions %d x %d (current cost: %d)\n", map.y_max, map.x_max, map.costs[3][4]);
    // for (int i = 0; i < map.y_max; i++)
    // {
    //     for (int j = 0; j < map.x_max; j++)
    //     {
    //         printf("%-5d ", map.costs[i][j]);
    //     }
    //     printf("\n");
    // }
    free(alphabet);
    return 0;
}