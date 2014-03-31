/*! 
 * @file    avl.h
 * @project AVL Tree - AT Computer Science
 * @author  Sam Redmond
 *   @date  March 02, 2014
 * @license MIT License 
 */

 /**
  * Header file for an AVL tree implementation in C. See the main C file for documentation
  */

#include <stdio.h>
#ifndef AVL_AVL_H //Include guard
#define AVL_AVL_H

/* MACROS */
#define TABS(depth) for (int x = 0; x < (depth); ++x){printf("\t");} //Print `depth` tabs
#define PRINT_LINE_SEP() printf("-----------------------\n"); //Print a line separator

/* TYPE DEFINITIONS */
//`Node` Struct - typical tree in our AVL tree
typedef struct node {
	char *word; //Word (key for BST property)
	int count; //How many instances of `word` have been inserted
	struct node *left; //Left Child
	struct node *right; //Right Child
	int height; //AVL height: 1 if leaf node, node depth otherwise
} Node;

//`Tree` Struct - just points to the root of the tree
typedef struct tree {
	struct node *root; //Pointer to the root which anchors the top of the tree
} Tree;

/* FUNCTION PROTOTYPES */
void instructions();

//Insert
void insert(Tree *tree, char *word);
Node* insertAux(Node* root, char *word);

//Search
Node* search(Tree *tree, char *word);
Node* searchAux(Node *root, char *word);

//Delete
void deleteFromTree(Tree *tree, char *word);
Node* deleteAux(Node *root, char *word, int forceful);

//UTILITIES
//AVL specific
Node* rotateLeft(Node *root);
Node* rotateRight(Node *root);
Node* rebalance(Node *root);

//Views
void view(Tree *tree);
void viewAux(Node *root, int depth);
void verboseView(Tree *tree);
void verboseViewAux(Node *root, int depth);

//Constructors (initializers)
Node* createNode(char *word);
Tree* createTree();

//Destroy
void destroy(Tree *tree);
void destroyAux(Node *root);

//Fetch user input
void getInt(int *ptr);
void getString(char **wordPtr);

//Miscellaneous
int height(Node *root);
int getBalance(Node *root);

Node* largestIn(Node *root);
Node* smallestIn(Node *root);

void printInOrder(Node *root);
int max(int a, int b);

//FILE IMPORTING
//Filesystem interaction (not done)
FILE* getFile();
void fileInstructions();
void ls();
void cd();
FILE* selectFile();
int importFile(FILE *fptr, Tree *tree);

//File selectors
int accept(struct dirent *dir);
int isDir(struct dirent *dir);
int isFile(struct dirent *dir);
int isTextFile(struct dirent *dir);

#endif