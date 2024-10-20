#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "map.h"
#include "loc.h"

char* arrayrandomproba() {
    char *arrayrandomproba = (char*)malloc(9 * sizeof(char));
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
    for (int i = 0; i < TB; i++) arrayloop[index++] = 'B';

    for (int i = 0; i < 9; i++) {
        int randomnumber = rand() % SIZE;
        arrayrandomproba[i] = arrayloop[randomnumber];

        if (arrayrandomproba[i] == 'A') F_10--;
        if (arrayrandomproba[i] == 'B') F_20--;
        if (arrayrandomproba[i] == 'C') F_30--;
        if (arrayrandomproba[i] == 'R') R_10--;
        if (arrayrandomproba[i] == 'T') TR--;
        if (arrayrandomproba[i] == 'L') TL--;
        if (arrayrandomproba[i] == 'B') TB--;

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
    int value; 
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

t_node *createNode(int val){
    t_node *node = malloc(sizeof(t_node));
    if (node == NULL) {
        fprintf(stderr, "Erreur d'allocation de mémoire\n");
        exit(1);
    }
    node->value = val;
    for (int i = 0; i < 9; i++)
        node->children[i] = NULL;
    return node;
}

void createTreeRecursive(t_node* parent, int depth, int maxDepth, int numChildren) {
    if (depth > maxDepth) {
        return;  
    }

    for (int i = 0; i < numChildren; i++) {
        t_node* child = createNode(0);
        parent->children[i] = child;
        createTreeRecursive(child, depth + 1, maxDepth, numChildren); 
    }
}

void createTree(char* move, t_localisation localisation, t_map map) {
    t_tree tree = createEmptyTree();
    int costRover = map.costs[localisation.pos.x][localisation.pos.y];
    t_node *root = createNode(costRover);
    tree.root = root;
    int maxDepth = 8;
    int numChildren = 9;

    createTreeRecursive(tree.root, 1, maxDepth, numChildren);
    displayTree(&tree);
}

void displayNode(t_node *node, int depth) {
    if (node == NULL) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("Value: %d\n", node->value);

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
    t_map map = createMapFromFile("../maps/example1.map");
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


    t_tree tree = createEmptyTree();
    t_node *root = createNode(5);
    tree.root = root;

    root->children[0] = createNode(10);
    root->children[1] = createNode(15);
    root->children[0]->children[0] = createNode(20);
    root->children[0]->children[1] = createNode(25);

    displayTree(&tree);


    char* result = arrayrandomproba();

    if (result != NULL) {
        printf("Generated array: ");
        for (int i = 0; i < 9; i++) {
            printf("%c", result[i]);
        }
        printf("\n");
        free(result);
    }

    return 0;
}
