#AVL Tree
This program is a C implementation of an AVL tree, a self-balancing binary search tree. A node's primary value is a `char* word`. A node also keeps track of how many instances of `word` have been inserted 


##Installation
First, clone the repository `git clone https://github.com/sredmond/c-AVL.git`. If you can execute the `avl` binary, wonderful! Otherwise, recompile the `avl.c` file using your favorite C compiler and flags to execute the new binary.

Note: the `avl` binary has been tested only on Mac OSX 10.6 and above. 

##Features
User interaction happens at a command line prompt, where an integer choice is entered corresponding to a specified action. 

 * The actions are:
 * Insert (1): The user is prompted for a word, which is subsequently inserted into the AVL tree. The tree will balance itself if necessary
 * Search (2): The user is prompted for a word, and the tree is searched for a node which contains that word. The user is notified of the results
 * Delete (3): The user is prompted for a word, and an instance of that word is removed from the tree, if possible
 * View (4): Displays the tree in a simple fashion
 * Verbose View (5): Displays the tree in a verbose manner, revealing information about pointers, struct sizes, and more.
 * Import File (6): Opens a low-level file browser command line prompt in which the user can navigate the computer's file system (using utilities that behave like `ls` and `cd`) to select a *.txt file for import
 * Exit (7): Quits the program after freeing all allocated memory

##Known Bugs
* Some operating systems do not support the `d_type` member of a `struct dirent`. Without a safe definition of this member, the program will either crash (oh no!), or return `DT_UNKNOWN` for all files. In the latter case, `cd` and `select` would not function correctly in the file explorer, but `ls` would still work. There is supposed to be a symbol `_DIRENT_HAVE_D_TYPE` from `<sys/dirent.h>` which is defined if and only if `struct dirent` has a well-defined `d_type` member. However, `struct dirent`s on my machine have a `d_type` despite `_DIRENT_HAVE_D_TYPE` being undefined, so the consistency of this symbol definition is unclear.
* Because the command line's read buffer is only 1024 bytes long, if the user enters 1024 characters and a newline, the newline will not be sent and thus will not unblock getchar()
* The select function uses naive string comparison to determine if a given file is a plain text file (if it ends in ".txt"). However, it is possible that another file (say a `*.pdf`) could be renamed as and converted to a `*.txt`, leading to a corrupt plain text file. This file would be available to import via the current implementation of select.
