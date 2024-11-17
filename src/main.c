#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "map.h"
#include "loc.h"
#include "moves.h"


void printPath(const char *prefix, const char *string) {
    printf("%s ", prefix);
    if (string != NULL)
        printf("%s", string);
    else 
        printf("(null)");
    printf("\n");
}

char* arrayrandomproba() {
    char *arrayrandomproba = (char*)malloc(6 * sizeof(char));
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

    for (int i = 0; i < 5; i++) {
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

    return arrayrandomproba;
}

typedef struct s_node
{
    struct s_node *children[9];
    char* path[5];
    int val;
} t_node;

typedef struct s_tree
{
    t_node *root;
} t_tree;

t_tree createEmptyTree(){
    t_tree temp;
    temp.root = NULL;
    return temp;
}

int calculate_node(char* t_path, t_localisation localisation, t_map map) {
    t_localisation phantomloc = localisation;
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
    if (phantomloc.pos.x < 0 || phantomloc.pos.x >= 10 || phantomloc.pos.y < 0 || phantomloc.pos.y >= 10) {
        printf("Erreur : Rover out of range\n");
        return 999999;  // Imposible Path
    }

    int cost = map.costs[phantomloc.pos.y][phantomloc.pos.x];
    printf("Position: (%d, %d), Orientation %d\n", phantomloc.pos.x, phantomloc.pos.y, phantomloc.ori);
    return cost;
}


t_node *createNode(char* t_path, t_localisation localisation, t_map map){
    t_node *node = malloc(sizeof(t_node));
    if (node == NULL) {
        fprintf(stderr, "Erreur: allocation de mémoire\n");
        exit(1);
    }

    int length = strlen(t_path);
    printf("all Move: %d\n", length);
    if (length > 6) {
        fprintf(stderr, "Erreur: trop d'éléments dans t_path\n");
        exit(1);
    }

    for(int i = 0; i < 5; i++){node->path[i] = t_path[i];}
    node->val = calculate_node(t_path, localisation, map);
    
    for (int i = 0; i < 9; i++){node->children[i] = NULL;}

    displayNode(node, 0);
    return node;
}

void createBranch(t_node *parent_node, int nChild, int depth, char* move, t_localisation localisation, t_map map){
    for(int i = 0; i<nChild; i++){
        // t_node manager
        int costRover = map.costs[localisation.pos.x][localisation.pos.y];
        char firstMove[6];
        // strncpy(firstMove, move, i);
        strncpy(firstMove, move, sizeof(firstMove) - 1);
        firstMove[sizeof(firstMove) - 1] = '\0';

        printPath("\nMove:", move);
        parent_node->children[i] = createNode(&move[i], localisation, map);

        for (int j = 0; j < 9; j++) {
            if (firstMove[i] == "\0") break;
            parent_node->children[i]->path[j] = malloc(2 * sizeof(char));
            if (parent_node->children[i]->path[j] == NULL) {
                fprintf(stderr, "Erreur allocation\n");
                exit(1);
            }
            strcpy(parent_node->children[i]->path[j], &firstMove[j]);
        }
        // parent_node->children[i]->val = 777;


        // depth manager
        if(depth == 4)
            return;
        createBranch(parent_node->children[i], nChild - 1, depth + 1, move, localisation, map);
    }
}

void createTree(char* move, t_localisation localisation, t_map map) {
    
    t_tree tree = createEmptyTree();
    int costRover = map.costs[localisation.pos.x][localisation.pos.y];
    t_node *root = createNode(move, localisation, map);
    tree.root = root;
    int nChild = 9;
    int depth = 1;
    createBranch(tree.root, nChild, depth, move, localisation, map);
    
    // Search Better path
    t_node *optimalNode = NULL;
    int minCost = 50;
    char* optimalPath = NULL;

    findOptimalPath(tree.root, &optimalNode, &minCost, &optimalPath);

    printf("\n\n### Final Best Path Rover Find ###\n");
    if (optimalPath != NULL) {
        printf("Optimal path: %s\n", optimalPath);
        printf("Cost: %d\n", minCost);
        printf("( ˶ˆᗜˆ˵ )\n");
    } else
        printf("Safly no path find...\n૮(˶ㅠ︿ㅠ)ა\n");
}

// Reste plus qu'a fix cette fonction car deja j'ai pas compris comment ca peut marcher et j'arrive pas a return tout le path dcp force a nous
void findOptimalPath(t_node* node, t_node** optimalNode, int* minCost, char** optimalPath) {
    if (node == NULL) return;

    if (node->val < *minCost) {
        *minCost = node->val;
        *optimalNode = node;
        *optimalPath = node->path;
    }

    for (int i = 0; i < 9; i++) {
        findOptimalPath(node->children[i], optimalNode, minCost, optimalPath);
    }
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

struct s_localisation calculate_new_location(struct s_localisation loc, t_node* node, char order, t_map map) {
    int distance = 0;
    int cost;

    switch (order) {
        case 'A':
            distance = 1;
            break;
        case 'B':
            distance = 2;
            break;
        case 'C':
            distance = 3;
            break;
        case 'R':
            distance = -1;
            break;
        case 'T':
            loc.ori = (loc.ori + 1) % 4;
            break;
        case 'L':
            loc.ori = (loc.ori - 1 + 4) % 4;
            break;
        case 'J':
            loc.ori = (loc.ori + 2) % 4;
            break;
        default:
            break;
    }

    switch (loc.ori) {
        case NORTH:
            loc.pos.y += distance;
            break;
        case SOUTH:
            loc.pos.y -= distance;
            break;
        case EAST:
            loc.pos.x += distance;
            break;
        case WEST:
            loc.pos.x -= distance;
            break;
        default:
            break;
    }

    cost = map.costs[loc.pos.x][loc.pos.y];
    node->val = cost;
    return loc;

/*
 * Faut regarder que l'on sorte pas du jeu et qu'on traverse pas de crevasse :)
 */
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


    // t_tree tree = createEmptyTree();
    // t_node *root = createNode(5);
    // tree.root = root;

    // root->children[0] = createNode(10);
    // root->children[1] = createNode(15);
    // root->children[0]->children[0] = createNode(20);
    // root->children[0]->children[1] = createNode(25);

    // displayTree(&tree);

    //char *result = (char*)malloc(9 * sizeof(char));
    char* result = arrayrandomproba();
    
    // if (result != NULL) {
    //     printf("Generated array: ");
    //     for (int i = 0; i < 5; i++) {
    //         printf("%c", result[i]);
    //     }
    //     printf("\n");
    //     //free(result);
    // }
    printPath("Generated array: ", result);

    createTree(result, loc, map);

    free(result);
    return 0;
}