#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stddef.h>
#include <errno.h>

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE -1
#endif

struct binary_tree_node
{
    void *bt_node_element;
    struct binary_tree_node *bt_node_left, *bt_node_right;
};

struct binary_tree
{
    struct binary_tree_node *bt_root;
    size_t bt_elementSize;
    int (*bt_compareFunction)(const void *, const void *);
};

//* FUNCTIONS TO WORK DIRECTLY ON THE NODES

/**
 * @brief Finds the minimum element in the binary tree.
 *
 * Traverses the binary tree to find the node with the minimum element, according to the binary search tree property.
 *
 * @return Pointer to the node containing the minimum element, or NULL if the tree is empty.
 */
struct binary_tree_node *bt_node_findMin(struct binary_tree_node *root);

/**
 * Dynamically allocates memory for a new binary tree node and its element, copying the element data into the new node.
 *
 * @return Pointer to the newly created node, or NULL if memory allocation fails.
 */
struct binary_tree_node *bt_node_createNode(const void *element, size_t elementSize);

/**
 * @brief Inserts an element into the binary tree.
 *
 * Recursively inserts a new element into the binary tree, maintaining the binary search tree property.
 *
 * @return Pointer to the inserted node, or NULL on failure.
 *
 * @warning from great power comes great responsibility. Failure means a malloc didn't work.
 */
struct binary_tree_node *bt_node_insert(struct binary_tree_node **root, const void *element, size_t elementSize, int(compare_function)(const void *, const void *));

/**
 * @brief Deletes a node with the specified element from the binary tree.
 */
struct binary_tree_node *bt_node_delete(struct binary_tree_node *root, const void *element, size_t elementSize, int (*compare_function)(const void *, const void *));

/**
 * @brief Searches for a node containing the specified element in the binary tree.
 *
 * Recursively searches for a node that contains the specified element, based on the provided comparison function.
 *
 * @return Pointer to the found node, or NULL if the element is not found.
 */
struct binary_tree_node *bt_node_search(struct binary_tree_node *root, const void *element, size_t elementSize, int (*compare_function)(const void *, const void *));

/**
 * @brief Visits each node of the binary tree in inorder sequence and applies a given function.
 *
 * Traverses the tree in inorder (left node, current node, right node) and applies the provided function to each node's element.
 *
 * @param visualizeElement Function to apply to each element during the visit.
 */
void bt_node_visitInOrder(struct binary_tree_node *root, void (*visualizeElement)(const void *));

/**
 * @brief Frees all nodes of the binary tree.
 *
 * Recursively frees the memory of all nodes in the binary tree, including their elements.
 *
 * @warning from great power comes great responsibility. Ensure proper use to avoid memory leaks or dangling pointers.
 */
void bt_node_freeTree(struct binary_tree_node *root);

//* FUNCTION TO WORK USING THE OBJECT BINARY_TREE

/**
 * @brief Initializes a new binary tree.
 *
 * Creates a binary tree with specified element size and comparison function, initializing the root to NULL.
 *
 * @return An initialized binary_tree structure.
 */
struct binary_tree bt_create(size_t elementSize, int (*compare_function)(const void *, const void *));

/**
 * @brief Inserts an element into the binary tree.
 *
 * Attempts to insert a new element into the binary tree, maintaining the binary search tree property.
 *
 * @return SUCCESS if the element is successfully inserted, FAILURE if malloc fails.
 */
int bt_insert(struct binary_tree *object, const void *element);

/**
 * @brief Deletes an element from the binary tree.
 */
void bt_delete(struct binary_tree *object, const void *element);

/**
 * @brief Searches for an element in the binary tree.
 *
 * @param element Pointer to the element to compare (using the object->bt_compareFunction) with the one you are looking for in the tree.
 *
 * @return Pointer to the found element, or NULL if the element is not found.
 */
void *bt_search(struct binary_tree *object, const void *element);

/**
 * @brief Visits each node of the binary tree in inorder sequence.
 *
 * Traverses the binary tree in inorder and applies a given function to the element of each visited node.
 *
 * @param visualizeElement Function to apply to each element during the visit.
 */
void bt_visitInOrder(struct binary_tree *object, void (*visualizeElement)(const void *));

/**
 * @brief Frees all nodes of the binary tree.
 *
 * Recursively frees the memory of all nodes in the binary tree, setting the root to NULL.
 * You can still use the freed tree for the same type of elements.
 *
 * @warning from great power comes great responsibility. Ensure proper use to avoid memory leaks.
 */
void bt_freeTree(struct binary_tree *object);

#endif