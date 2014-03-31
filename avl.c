/*! 
 * @file    avl.c
 * @project AVL Tree - AT Computer Science
 * @author  Sam Redmond
 *   @date  March 02, 2014
 * @license MIT License 
 */

/**
 * AVL Tree
 * This program is a C implementation of an AVL tree, a self-balancing binary search tree. A node's primary value is a char* word. A node also keeps track of how many instances of `word` have been inserted 
 * User interaction happens at a command line prompt, where an integer choice is entered corresponding to a specified action. 
 * 
 * The actions are:
 * • Insert (1): The user is prompted for a word, which is subsequently inserted into the AVL tree. The tree will balance itself if necessary
 * • Search (2): The user is prompted for a word, and the tree is searched for a node which contains that word. The user is notified of the results
 * • Delete (3): The user is prompted for a word, and an instance of that word is removed from the tree, if possible
 * • View (4): Displays the tree in a simple fashion
 * • Verbose View (5): Displays the tree in a verbose manner, revealing information about pointers, struct sizes, and more.
 * • Import File (6): Opens a low-level file browser command line prompt in which the user can navigate the computer's file system (using utilities that behave like `ls` and `cd`) to select a *.txt file for import
 * • Exit (7): Quits the program after freeing all allocated memory
 *
 * Features
 * Too many to count! :) Try to find them all. Among the highlights - a file importing UI that interfaces with the filesystem, arbitrary length string reading, and lots of error catching. Memory management is a plus too. And, of course, a friendly UI allows users to interact well with the tree. There are two view functions and a debugging feature. The code is also organized intelligently. 
 *
 * And, of course, there is the implementation of an AVL tree, with support for insert, search, and delete. 
 *  
 * Known Bugs
 * • Some operating systems do not support the d_type member of a struct dirent. Without a safe definition of this member, the program will either crash, or return DT_UNKNOWN for all files. In the latter case, `cd` and `select` would not function correctly, but `ls` would still work. There is supposed to be a symbol `_DIRENT_HAVE_D_TYPE` from <sys/dirent.h> which is defined iff struct dirent has a well-defined d_type member. However, struct dirents on my machine have a d_type despite _DIRENT_HAVE_D_TYPE being undefined, so its unclear the consistency of this symbol definition
 * • Not so much a bug as a limitation of Terminal's command line interface: Terminal's read buffer is only 1024 bytes long, so if the user enters 1024 characters and a newline, the newline will not be sent and thus will not unblock getchar()
 * • The select function uses naive string comparison to determine if a given file is a plain text file (if it ends in ".txt"). However, it is possible that another file (say a "*.pdf") could be renamed as and converted to a "*.txt", leading to a corrupt plain text file. This file would be available to import via the current implementation of select
 *
 * ENJOY
 * --Sam
 */

//Required headers to include
#include <stdio.h> /* printf, scanf, getchar, fscanf, FILE */
#include <stdlib.h> /* malloc, realloc, free, exit */
#include <string.h> /* strlen, strcpy, strcmp, strncmp */
#include <ctype.h> /* isalpha, isspace */
#include <dirent.h> /* struct dirent, alphasort, scandir, [DT_TYPES] */
#include <unistd.h> /* getcwd, chdir */
#include "avl.h" /* Everything else */

/**
 * TODO
 * Better support for character types when reading files
 * 50/50 on delete
 * ----------*/

//Verbose output for debugging
#define VERBOSE 0

int main()
{
  //We have one instance of a Tree which anchors our AVL tree
  Tree* tree = createTree();
  
  //Print instructions
  PRINT_LINE_SEP();
  instructions();
  PRINT_LINE_SEP()
  
  //Allocate space for persistent trackers
  int choice = 0;
  char *word = (char *) malloc(4); //Arbitrary starting length
  Node* res = 0;
  FILE* fptr;

  //Main loop
  do
  {
    //Get a command from the user
    printf("Enter a command: ");
    getInt(&choice);
    PRINT_LINE_SEP()

    //Options
    switch (choice)
    {
      case 1: //Insert
        printf("Enter an word to insert into the AVL tree: ");
        getString(&word);
        insert(tree, word); //Insert into tree
        break;
      case 2: //Search
        printf("Enter a word to search for in the AVL tree: ");
        getString(&word);
        res = search(tree, word); //Search in tree
        if (res) //We found it!
        {
          printf("Found %d instance%s of '%s'\n", res->count, (res->count == 1) ? "" : "s", res->word);
          printf("Node is at %p\n", (void*) res);
          if (VERBOSE) 
          {
            printf("Pointer to node is at %p\n", (void*) &res);
          }
        }
        else
        {
          printf("Word not found.\n");
        }
        break;
      case 3: //Delete
        printf("Enter a word to delete from the AVL tree: ");
        getString(&word);
        deleteFromTree(tree, word); //Delete from tree
        break;
      case 4: //View
        view(tree);
        break;
      case 5: //Verbose view
        verboseView(tree);
        break;
      case 6: //Import file
        fptr = getFile(); //NULL if user cancelled out of getFile system
        if (fptr) //Did the user select a file?
        {
          importFile(fptr, tree); //Import it
        }
        break;
      case 7: //Exit
        printf("Exiting...\n");
        break;
      default: //Invalid command
        printf("Invalid command!\n");
        instructions();
    }
    PRINT_LINE_SEP();
  } while (7 != choice);
  
  //Clean up memory
  destroy(tree);
  free(word);
  
  return 0; //Successful execution
}

//Print instructions for operation
void instructions()
{
  printf("To insert a value: press 1\n"
    "To locate a value: press 2\n"
    "To delete a value: press 3\n"
    "To view: press 4\n"
    "To view verbosely: press 5\n"
    "To import a *.txt file: press 6\n"
    "To exit: press 7\n");
}

/* ------
 * INSERT
 * ------ */

/* Insert a word into the AVL tree, such that the AVL balance property is maintained
 * Precondition: tree is not null, word is not null */
void insert(Tree *tree, char *word)
{
  if (tree->root) //At least one node exists
  {
    tree->root = insertAux(tree->root, word);
  }
  else //First node we're inserting
  {
    tree->root = createNode(word);
  }
}

/* Perform a standard BST insert of `word` into `root`, with the addition that the tree self-balances according to the AVL property. In the process of balancing, the root node may change, so we return the new root that anchors the new, balanced subtree after `word` has been inserted
 * The back propagation normally required of an AVL insert is handled by the recursive function calls which hand priority up the tree from the leaf to the root, rebalancing at every node along the way
 * Precondition: word is not null
 * Return: Pointer to the node that anchors the new subtree
*/
Node* insertAux(Node* root, char *word)
{
  if (root)
  {
    int cmp = strcmp(word, root->word); 
    if (cmp > 0) //Word is `larger than` root->word. Descending right..
    {
      root->right = insertAux(root->right, word); //Insert `word` into root's right subtree, and reassign its right child to be the new root of this subtree
    }
    else if (cmp < 0) //Word is `smaller than` root->word. Descending left..
    {
      root->left = insertAux(root->left, word); //Insert `word` into root's left subtree, and reassign its left child to be the new root of this subtree
    }
    else //We found an already-existing node with this value, so increment it's count
    {
      (root->count)++;
    }
    
    //Update height
    root->height = max(height(root->left), height(root->right)) + 1;

    //Rebalance
    return rebalance(root);
  }
  else //By following BST edges, we've reached either the left of the right of an existing leaf
  {
    return createNode(word);
  }
}

/* ------
 * SEARCH
 * ------ */

/* Perform a standard BST search for a given value
 * Precondition: tree is not null, word is not null
 * Return: Pointer to Node with value `word` if found, else NULL */
Node* search(Tree *tree, char *word)
{
  if (tree->root) //At least one node
  {
    return searchAux(tree->root, word);
  }
  else //Empty tree
  {
    return NULL;
  }
}

/* Auxiliary search
 * Precondition: root->word is not null, word is not null
 * Return: Pointer to Node with value `word` if found, else NULL */
Node* searchAux(Node *root, char *word)
{
  if (root)
  {
    int cmp = strcmp(word, root->word);
    if (cmp > 0) //Word is `larger than` root->word. Descending right...
    {
      return searchAux(root->right, word);
    }
    else if (cmp < 0) //Word is `smaller than` root->word. Descending left...
    {
      return searchAux(root->left, word);
    }
    else //We found it! root->word == word
    {
      return root;
    }
  }
  return NULL;
}

/* ------
 * DELETE
 * ------ */

/* Deletes a given value from the AVL tree if possible while maintaining AVL structure 
 * Precondition: tree is not null, word is not null
 */
void deleteFromTree(Tree *tree, char *word)
{
  if (tree->root) //Some nodes exists
  {
    tree->root = deleteAux(tree->root, word, 0);
  }
  else //Empty
  {
    printf("Tree is empty. Cannot remove '%s'.\n", word);
  }
}

/* Deletes a word from root's subtree, while maintaining AVL balance in the subtree
 * As with insert, the balancing is handled by recursive function calls as they return in the stack from leaf to root
 * If the parameter forceful is nonzero, the count of a node is ignored and an entire node is deleted
 * Precondition: word is not null
 * Return: Pointer to the node that anchors the new subtree
 */ 
Node* deleteAux(Node *root, char *word, int forceful)
{
  if (root)
  {
    int cmp = strcmp(word, root->word);
    if (cmp > 0) //Word is `larger than` root->word. Descending right...
    {
      root->right = deleteAux(root->right, word, forceful); //Delete the word from the right subtree
    }
    else if (cmp < 0) //Word is `smaller than` root->word. Descending left...
    {      
      root->left = deleteAux(root->left, word, forceful); //Delete the word from the left subtree
    }
    else //Found the node we're trying to delete
    {
      if (root->count == 1 || forceful) //We're going to delete the entire node
      {
        if (root->left)
        {
          if (root->right) //Both left and right children exist
          {
            int path = rand() % 2; //We'll either promote from the right or from the left
            if (path) //Promote from the right
            {
              //Inorder successor
              Node* temp = smallestIn(root->right);

              // Copy the inorder successor's data to this node
              root->word = temp->word;
              root->count = temp->count;

              // Delete the inorder successor forcefully
              root->right = deleteAux(root->right, temp->word, 1);  
            }
            else //Promote from the left
            {
              //Inorder predecessor
              Node* temp = largestIn(root->right);

              // Copy the inorder predecessor's data to this node
              root->word = temp->word;
              root->count = temp->count;

              // Delete the inorder predecessor forcefullt
              root->left = deleteAux(root->left, temp->word, 1);  
            }
            
          }
          else //Only left child exists
          {
            //Promote root's left child
            Node *left = root->left;
            free(root);
            return left;
          }
        }
        else
        {
          if (root->right) //Only right child exists
          {
            //Promote root's right child
            Node *right = root->right;
            free(root);
            return right;
          }
          else //No child exists
          {
            return NULL;
          }
        }
      }
      else //There was more than one instance of word inserted into the tree, so decrement root's count
      {
        (root->count)--;
        return root;
      }
    }

    //Update height
    root->height = max(height(root->left), height(root->right)) + 1;

    //Rebalance
    return rebalance(root);
  }
  else //We traversed the tree but never found our word. It must not be in the binary tree
  {
    printf("%s not found. Unable to delete.\n", word);
    return root;
  }
}
 
/* Rotate a tree left rooted by root
 *    X             Y
 *   / \           / \
 *  T1  Y   -->   X  T3
 *     / \       / \
 *    T2 T3     T1 T2
 *
 * T1, T2, and T3 are subtrees
 * The BST condition is maintained in both trees, since T1 < X < T2 < Y < T3
 * Precondition: root is not null, root has a non-null right child
 * Return: the new root of the subtree (Y)
 */
Node* rotateLeft(Node *root)
{
  Node *rightChild = root->right; //Y
  Node *temp = rightChild->left; //T2

  //Perform rotation
  rightChild->left = root; //Y's left child is X
  root->right = temp; //X's right child is root

  //Update height of X, then Y
  root->height = max(height(root->left), height(root->right)) + 1;
  rightChild->height = max(height(rightChild->left), height(rightChild->right)) + 1;

  return rightChild; //The new root of the subtree (Y)
}

/* Rotate a tree right rooted by root (similar to rotateLeft)
 *      Y             X
 *     / \           / \
 *    X  T3   -->   T1  Y
 *   / \               / \
 *  T1  T2            T2 T3
 *
 * T1, T2, and T3 are subtrees
 * The BST condition is maintained in both trees, since T1 < x < T2 < y < T3
 * Precondition: root is not null, root has a non-null left child
 * Return: the new root of the subtree (X)
 */
Node* rotateRight(Node *root)
{
  Node *leftChild = root->left; //X
  Node *temp = leftChild->right; //T2

  //Perform rotation
  leftChild->right = root; //X's right child is Y
  root->left = temp; //Y's left child is T2
  
  //Update height of Y, then X
  root->height = max(height(root->left), height(root->right)) + 1;
  leftChild->height = max(height(leftChild->left), height(leftChild->right)) + 1;

  return leftChild; //The new root of the subtree (X)
}

/* Rebalance roots subtree, and return the new root of the balanced subtree
 * AVL rebalancing involves four cases: left-left, right-right, left-right, and right-left unbalanced. 
 * T1, T2, T3 and T4 are subtrees.

  Left Left Case
         z                                     y 
        / \                                  /   \
       y   T4        rotateRight(z)         x     z
      / \          ----------------->      / \   / \ 
     x   T3                               T1 T2 T3 T4
    / \
  T1   T2

  Right Right Case
     z                                         y
    / \                                      /   \ 
  T1   y             rotateLeft(z)          z     x
      / \          ---------------->       / \   / \
    T2   x                                T1 T2 T3 T4
        / \
      T3   T4

  Left Right Case
       z                                       z                                      x
      / \                                     / \                                   /   \ 
     y   T4          rotateLeft(y)           x   T4          rotateRight(z)        y     z
    / \            ---------------->        / \            ----------------->     / \   / \
  T1   x                                   y   T3                                T1 T2 T3 T4
      / \                                 / \
    T2   T3                             T1   T2
  
  Right Left Case
     z                                         z                                      x
    / \                                       / \                                   /   \ 
  T1   y             rotateRight(y)         T1   x           rotateLeft(z)         z     y
      / \          ----------------->           / \        ---------------->      / \   / \
     x   T4                                   T2   y                             T1 T2 T3 T4
    / \                                           / \
  T2   T3                                       T3   T4

 * Precondition: the absolute value of root's balance factor is 2 or less
 * Return: the new root of the balanced subtree
 */
Node* rebalance(Node *root)
{
  int balance = getBalance(root); //Did this node become unbalanced?
  //Left Left Case
  if (balance > 1 && getBalance(root->left) > 0) //Balance > 1 guaranteers root->left exists
  {
    if (VERBOSE) {
      printf("Left-Left unbalanced: Rotating '%s' right.\n", root->word);
    }
    return rotateRight(root);
  }
  // Right Right Case
  else if (balance < -1 && getBalance(root->right) < 0) //Balance < -1 guaranteers root->right exists
  {
    if (VERBOSE) {
      printf("Right-Right unbalanced: Rotating '%s' left.\n", root->word);
    }
    return rotateLeft(root);
  }
  // Left Right Case
  else if (balance > 1 && getBalance(root->left) < 0) //Balance > 1 guaranteers root->left exists
  {
    if (VERBOSE) {
      printf("Left-Right unbalanced at '%s': Rotating '%s' left, then rotating '%s' right.\n", root->word, root->left->word, root->word);
    }
    root->left =  rotateLeft(root->left);
    return rotateRight(root);
  }
  // Right Left Case
  else if (balance < -1 && getBalance(root->right) > 0) //Balance < -1 guaranteers root->right exists
  {
    if (VERBOSE) {
      printf("Right-Left unbalanced at %s: Rotating '%s' right, then rotating '%s' left.\n", root->word, root->right->word, root->word);
    }
    root->right = rotateRight(root->right);
    return rotateLeft(root);
  }
  //If we didn't enter any of the above cases, the node was already balanced, and we return the (unchanged) node pointer
  return root;
}

/* -----
 * VIEWS
 * ----- */

/* Shows the contents of the current AVL tree in a simple display
 * Precondition: tree is not null */
void view(Tree *tree)
{
  if (tree->root) //Nonempty
  {
    viewAux(tree->root, 0); //View a simple display of the root of the AVL tree, starting at depth 0
  }
  else
  {
    printf("Empty.\n");
  }
}

//Traverse root's subtree in reverse in-order, displaying root's word and count padded with `depth` tabs
void viewAux(Node *root, int depth)
{
  if (root)
  {
    //Recursion
    viewAux(root->right, depth + 1); //Right subtree
    TABS(depth); //Pad with tabs
    printf("%s(%d)\n",root->word, root->count); //root's information
    viewAux(root->left, depth + 1); //Left subtree
  }
}

/* Shows the contents of the current AVL tree in a verbose display
 * Precondition: tree is not null */
void verboseView(Tree *tree)
{
  //Print information about pointer locations
  printf("Pointer to tree is at %p in the stack\n", (void*) &tree);
  printf("Tree is at %p in the heap\n", (void*) tree);
  printf("Tree's root is at %p in the heap\n", (void*) tree->root);
  printf("A Tree has size %lu bytes | A Node has size %lu bytes\n", sizeof(Tree), sizeof(Node)); //Struct sizes
  PRINT_LINE_SEP()
  if (tree->root) //Nonempty
  {
    verboseViewAux(tree->root, 0); //View a verbose display of the root of the AVL tree, starting at depth 0
    PRINT_LINE_SEP()

    //Just for fun, print the tree in order
    printf("In order, %s:", (tree->root->right || tree->root->left) ? "words are" : "word is"); //Do we have more than one word?
    printInOrder(tree->root); //Print the tree in order
    printf("\n");
  }
  else
  {
    printf("Empty.\n");
  }
}

//Traverse root's subtree in reverse in-order, displaying everything important about root padded with `depth` tabs
void verboseViewAux(Node *root, int depth)
{
  if (root) //Recursion
  {
    verboseViewAux(root->right, depth + 1); //Right subtree

    //Print all the information we know about root
    TABS(depth);
    printf("|Node[word=%s,count=%d,height=%d,balanceFactor=%d]\n",root->word, root->count, root->height, getBalance(root));
    TABS(depth);
    printf("|Node is at %p\n", (void*) root); //Root's location in the heap
    TABS(depth);
    printf("|Left child is at %p\n", (void*) root->left); //Left child
    TABS(depth);
    printf("|Right child is at %p\n", (void*) root->right); //Right child    
    
    verboseViewAux(root->left, depth + 1); //Left subtree
  }
}

/* ------------
 * CONSTRUCTORS
 * ------------ */

/* Default Java-esque constructor for a node
 * word: pointer to the first character of a null-terminated array of chars */
Node* createNode(char *word)
{
  //Ask malloc for a pointer to open space where we can build a node
  Node* temp = (Node*) malloc(sizeof(Node));
  if (!temp) //If malloc couldn't find any space, we're screwed
  {
    printf("Not enough heap space to construct a new node. Exiting...\n");
    exit(1);
  }
  int len = strlen(word);
  temp->word = (char*) malloc(len + 1); //Add a space for terminating \0
  if (!temp->word) //Couldn't find space? Oh no!
  {
    printf("Not enough heap space for %d characters. Exiting...\n", len + 1);
    exit(1);
  }
  strcpy(temp->word, word); //Copy the contents of the given word into the node's word
  temp->count = 1; //A new node has one word in it
  temp->left = temp->right = NULL; //Initialize left and right children to null
  temp->height = 1; //Added as a leaf
  return temp;
}

//Default Java-esque constructor for a tree
Tree* createTree()
{
  //Ask malloc for a pointer to open space where we can build our tree
  Tree *temp = (Tree*) malloc(sizeof(Tree));
  if (!temp) //If malloc couldn't find any space, we're screwed
  {
    printf("Not enough heap space to construct a new tree. Exiting...\n");
    exit(1);
  }
  temp->root = NULL; //Ensure that the tree's root is set to null
  return temp;
}

/* -------
 * DESTROY
 * ------- */

/* Destructor for tree
 * Precondition: tree is not null */
void destroy(Tree* tree)
{
  destroyAux(tree->root); //Destroy any nodes anchored to this tree (ok if tree->root is null)
  free(tree);
}

/* Destroy a given root by destroying its left child and right child (recursively),
 * then freeing its contents, and then itself */
void destroyAux(Node *root)
{
  if (root)
  {
    destroyAux(root->left); //Destroy left child
    destroyAux(root->right); //Destroy right child
    free(root->word); //Free node's word
    free(root); //Free node
  }
}

/* ----------------
 * FETCH USER INPUT
 * ---------------- */

//Get an integer from the user, handling any errors along the way
void getInt(int *ptr)
{
  while (!scanf("%d", ptr)) //scanf returns the number of pointers it filled
  {
    printf("That wasn't even an integer. Try again: ");
    while (getchar() != '\n'); //Move the read pointer along
  }
  while (getchar() != '\n'); //Flush the input buffer
}

/* Get a string from the user, and store it in *wordPtr. The returned characters are all graphable (0x21-0x7E). Consistent with scanf, non-graphable characters are ignored up until the first graphable character. Characters are added to a dynamically sized buffer until any whitespace character is encountered, at which point the existing array of characters is null terminated
 * The most remarkable thing about this function is that is allows for arbitrary length strings to be entered using realloc(), which resizes (and perhaps moves) the memory block associated with a given pointer and copies the contents over, freeing the old contents. 
 //Precondition: wordPtr is a pointer to a char* which has been allocated with either malloc, realloc, or calloc (otherwise realloc won't know the size of the memory block)
 */
void getString(char** wordPtr)
{
  int length = strlen(*wordPtr) + 1; //We know *wordPtr has been malloc-ed at least this much space
  char *temp = NULL; //Landing zone for pointer to reallocated space
  
  int c = '\n'; //Current character
  size_t max_size = length;
  int i = 0; //Position in string
  while (!isgraph(c = getchar())); //Trash all the non-graphable characters
  do
  {
    (*wordPtr)[i++] = (char) c; //Append a character
    if (i == max_size) //We've run out of space!
    {
      max_size *= 2;
      temp = (char*) realloc(*wordPtr, max_size); //Reallocate space
      if (!temp) //No heap space could be found
      {
        free(*wordPtr);
        printf("Not enough space to reallocate the character buffer. Exiting...\n");
        exit(1);
      }
      *wordPtr = temp; //Reassign the char*
    }
  } while (!isspace(c = getchar())); //Loop until we see a whitespace character

  //Null terminate the string
  (*wordPtr)[i] = 0;
  
  if (c != '\n')
  {
    while (getchar() != '\n'); //Flush the input buffer  
  }
}

/* -------------
 * MISCELLANEOUS
 * ------------- */

//Gets height of a node, with the default value of 0 if the node is null
 int height(Node *root)
{
  return (root != NULL) ? root->height : 0;
}

//Gets balance factor of a node, with the default value of 0 is the node is null
int getBalance(Node *root)
{
  return (root != NULL) ? (height(root->left) - height(root->right)) : 0;
}

/* Get the largest node in a root's subtree by descending right as far as possible
 * Precondition: root is non-null */
Node* largestIn(Node *root)
{
  if (root->right) //There exists something larger than me
  {
    return largestIn(root->right); //Look in its subtree
  }
  return root;
}

/* Get the smallest node in a root's subtree by descending left as far as possible
 * Precondition: Root is non-null */
Node* smallestIn(Node *root)
{
  if (root->left)
  {
    return smallestIn(root->left);
  }
  return root;
}

/* Print a tree in order. Note that the printed text begins with a leading space.
 * root: the root of the subtree to print */
void printInOrder(Node *root)
{
  if(root) //End condition
  {
    printInOrder(root->left); //Recurse on left subtree
    printf(" %s", root->word); //Print root
    printInOrder(root->right); //Recurse on right subtree
  }
}

/* Simple utility returns maximum of two integers
 * Logic: a if a > b else b */
int max(int a, int b)
{
  return (a > b) ? a : b;
}

/* --------------
 * FILE IMPORTING
 * -------------- */

/** Information about filesystems in C
 * The basic unit of directory navigation in C is the struct dirent (a directory entry), defined in <dirent.h>. This struct has many members, but we only care about two: d_name, a null-terminated character array representing the entry name component; and d_type, an unsigned char that, if supported by the filesystem, takes on the value of DT_REG (file), DT_DIR (directory), DT_UNKNOWN (unknown), or other less likely values. 
 * 
 * See http://www.gnu.org/software/libc/manual/html_node/Accessing-Directories.html for more specific information about library functions
 */

/**
 * Basic file input */
FILE* getFile()
{
  //Print instructions
  printf(
    "#########################################################\n"
    "# Welcome to a low-level file navigation and input system\n"
    "# Basic UNIX-esque functions are provided\n"
    "#   ls: lists the files in the current working directory\n"
    "#   cd: change the current working directory\n"
    "#   select: choose a *.txt file (from the current working directory) to import into the AVL tree\n"
    "#   exit: exits this file browser\n"
    "#########################################################\n");
  PRINT_LINE_SEP();
  fileInstructions();

  //Persistent trackers
  int cmd = 0;
  FILE* res = NULL;

  while (4 != cmd) //Loop until user cancels or selects a file
  {
    //Print current working directory
    PRINT_LINE_SEP();
    char *pwd = getcwd(NULL, 0); 
    printf("# PWD: %s\n", pwd);

    //Get a command from the user
    printf("# Enter a command: ");
    getInt(&cmd);

    //Options
    switch (cmd)
    {
      case 1: //ls
        ls();
        break;
      case 2: //cd
        cd();
        break;
      case 3: //Select
        res = selectFile();
        if (res) //Did the user select a file? If so, return it; otherwise, continue looping
        {
          return res;
        }
        break;
      case 4: //Exit
        printf("# Exiting file browser...\n");
        break;
      default: //Invalid Command
        printf("# Invalid command!\n");
        fileInstructions();
    }
  }
  return NULL;
}

//Print instructions for the file navigator
void fileInstructions()
{
    printf("# ls: 1\n"
    "# cd: 2\n"
    "# select: 3\n"
    "# exit: 4\n");
}
//List the directory entries in the current working directory (similar to unix `ls` utility)
void ls()
{
  struct dirent **eps = NULL; //Pointer to an array of entry pointers
  struct dirent *fp = NULL; //Tracker for a particular directory entry

  //scandir populates an array of pointers to struct dirents, filtered by the accept function and sorted by alphasort, drawn from the directory entries associated with the given filename. It returns the number of entries put into the array of pointers
  int n = scandir ("./", &eps, accept, alphasort);
  
  if (n > 0) //Found some directory entries
  {
    int cnt;
    for (cnt = 0; cnt < n; ++cnt) //Iterate over the directory entries
    {
      fp = eps[cnt];
      int dt = fp->d_type; //Type of directory entry
      
      //Print information about directory entry
      printf("# %s", fp->d_name); //dirent name
      switch (dt)
      {
        case DT_REG: //Regular file
          printf(" - file\n");
          break;
        case DT_DIR: //Directory
          printf(" - directory\n");
          break;
        case DT_FIFO: //Named pipe (FIFO)
          printf(" - named pipe (FIFO)\n");
          break;
        case DT_SOCK: //Socket
          printf(" - local domain socket\n");
          break;
        case DT_CHR: //Character device
          printf(" - character device\n");
          break;
        case DT_BLK: //Block device
          printf(" - block device\n");
          break;
        case DT_LNK: //Symbolic link
          printf(" - symbolic link\n");
          break;
        default: //DT_UNKNOWN = unknown file type
          printf(" - unknown file type\n");
          break;
      }
    }
    free(fp);
    free(*eps);
  }
  else if (0 == n) //Should never enter here, because of . and .. always exist
  {
    printf("# No directory entries found.\n");
    free(*eps);
  }
  else //Also should never happen
  {
    printf("# Couldn't open the directory!\n");
  }
}

/* Change the current working directory, similar to the unix `cd` utility. 
 * Note: the . directory is always available, so if the user mistakenly enters this function, they can exit with no change to the current working directory by selecting .
 */
void cd()
{
  struct dirent **eps; //Pointer to an array of entry pointers
  struct dirent *fp; //Tracker for a particular directory entry

  //scandir populates an array of pointers to struct dirents, filtered by the isDir function and sorted by alphasort, drawn from the directory entries associated with the given filename. It returns the number of entries put into the array of pointers
  int n = scandir ("./", &eps, isDir, alphasort);
  int choice = 0;
  if (n > 0) //Found some subdirectories
  {
    //Display options
    int cnt;
    for (cnt = 0; cnt < n; ++cnt)
    {
      fp = eps[cnt]; //Given: fp->d_type == DT_DIR by our choice of filter
      printf("# %d: %s\n", cnt, fp->d_name);
    }

    //Get choice from user
    printf("# Enter the number of the directory to move to: ");
    getInt(&choice);
    while (choice < 0 || choice >= n)
    {
      printf("# The number must be between 0 and %d, inclusive.\n", n-1);
      printf("# Try again: ");
      getInt(&choice);
    }
    fp = eps[choice];
    
    //Try to change directory
    if (chdir(fp->d_name) == -1) //fp->d_name is a relative file path
    {
      printf("# Unable to change directory.\n");
    }
    free(fp);
    free(*eps);
  }
  else if (0 == n) //No subdirectories found
  {
    printf("# No directories found.\n"); //This should never happen due to the existence of . and ..
    free(*eps);
  }
  else
  {
    printf("# Couldn't open the directory!\n");
  }
}

FILE* selectFile()
{
  struct dirent **eps; //Pointer to an array of entry pointers
  struct dirent *fp; //Tracker for a particular directory entry

  //scandir populates an array of pointers to struct dirents, filtered by the isTextFile (files whose name ends with '.txt') function and sorted by alphasort, drawn from the directory entries associated with the given filename. It returns the number of entries put into the array of pointers
  int n = scandir ("./", &eps, isTextFile, alphasort);
  
  int choice = 0;
  if (n > 0) //Found some subdirectories 
  {
    //Display options
    printf("# -1: Cancel\n");
    int cnt;
    for (cnt = 0; cnt < n; ++cnt) //Iterate over *.txt files found
    {
      fp = eps[cnt]; //Given: fp->d_type == DT_DIR by our choice of filter
      printf("# %d: %s\n", cnt, fp->d_name);
    }
    //Get choice from user
    printf("# Enter the number of the file to import: ");
    getInt(&choice);
    while (choice < -1 || choice >= n)
    {
      printf("# The number must be between 0 and %d, inclusive, or -1 to cancel\n", n-1);
      printf("# Try again: ");
      getInt(&choice);
    }

    if (choice == -1) //Cancel
    {
      return NULL;
    }

    fp = eps[choice]; //Valid choice

    //Construct the full filepath
    char *pwd = getcwd(NULL, 0); //pwd
    char fpath[1024]; //Max length of file paths on DARWIN systems
    sprintf(fpath, "%s/%s", pwd, fp->d_name); //Paste the strings together

    FILE* fptr = fopen(fpath, "r"); //Open the file for reading
    if (!fptr)
    {
      printf("# Could not open file.\n");
    }
    return fptr; //Could be null
  }
  else if (0 == n) //No text files
  {
    printf("# No *.txt files in directory\n");
  }
  else //Alternate error
  {
    printf("# Couldn't open the current directory!\n");
  }
  return NULL;
}

/* Import all words from a file
 * Precondition: fptr is not null and opened for reading, tree is not null
 * For non-corrupted input, fptr should point to a file with whitespace-separated strings of alphabetic characters (all non-alphabetic characters are treated as delimiters)
 * Strings of arbitrary size are accepted, since the char* landing zone is dynamically reallocated more space if needed. 
 */
int importFile(FILE* fptr, Tree *tree)
{
  int max_length = 80; //Default buffer size
  int c = EOF;
  do
  {
    char *word = (char*) malloc(max_length); //Char* that accumulates our incoming characters
    char *temp = NULL; //Landing zone for pointer to reallocated space
    size_t current_size = max_length;
    if (word) //Malloc succeeded
    {
      int i = 0;
      while (isalpha(c = fgetc(fptr))) //Non-alphabetic characters function are treated as delimiters
      {
        word[i++] = (char) c;
        if (i == current_size) //Out of space! We need to reallocate
        {
          current_size *= 2;
          temp = (char*) realloc(word, current_size * sizeof(char));
          if (!temp) //Realloc failed to find heap space
          {
            free(word);
            printf("# Not enough heap space to reallocate the line buffer. Exiting...\n");
            free(tree);
            fclose(fptr);
            exit(1);
          }
          word = temp;
        }
      }
      if (i) //Non-null word
      {
        //Null terminate the string
        word[i] = 0;
        insert(tree, word);
        if (VERBOSE)
        {
          printf("# Adding: %s\n", word);
          view(tree);
          PRINT_LINE_SEP()
        }
        free(word);
      }
    }
    else
    {
      printf("# Not enough heap space to allocate the line buffer. Exiting...\n");
      free(tree);
      fclose(fptr);
      exit(1);
    }
  } while (c != EOF); //While we're not at the end of the file
  fclose(fptr); //Close the file
  if (VERBOSE)
  {
    printf("# Successfully read from file.\n");
  }
  return 0;
}

/* FILE SELECTORS
 * File selector functions are used to filter directory entries from the output of scandir. A given directory entry is ignored if the selector returns 0 
 */

//Accept every directory entry
int accept(struct dirent *dir)
{
  return 1; //Always nonzero
}

/*
 * Only accept directory entries whose d_type is DT_DIR, corresponding to a directory.
 * WARNING: On filesystems with no definition of d_type or where d_type is DT_UNKNOWN for all directory entries, this function will always return 0 (or crash)
 */
int isDir(struct dirent *dir)
{
  return (dir->d_type == DT_DIR) ? 1 : 0;
}

/*
 * Only accept directory entries whose d_type is DT_REG, corresponding to a file.
 * WARNING: On filesystems with no definition of d_type or where d_type is DT_UNKNOWN for all directory entries, this function will always return 0 (or crash)
 */
int isFile(struct dirent *dir)
{
  return (dir->d_type == DT_REG) ? 1 : 0;
}

/*
 * Only accept directory entries for which d_type is DT_REG and d_name ends with ".txt", which usually corresponds with a plain text file
 * WARNING: On filesystems with no definition of d_type or where d_type is DT_UNKNOWN for all directory entries, this function will always return 0 (or crash).
 * WARNING: Files can be converted from one extension to another. If one file (say a "*.pdf") is converted to a "*.txt", the plain text file will be corrupted, but this file selector will still accept it
 */
int isTextFile(struct dirent *dir)
{
  if (dir->d_type == DT_REG)
  {
    char *name = dir->d_name; //Name associated with directory entry
    size_t str_len = strlen(name);
    if(4 > str_len) //Can't possibly end in *.txt
      return 0;
    return 0 == strncmp(name + str_len - 4, ".txt", 4); //Are the last four letters of name equal to ".txt"?
  }
  return 0;
}