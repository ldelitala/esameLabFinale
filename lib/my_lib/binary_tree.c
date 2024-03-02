#include "../../include/my_lib/binary_tree.h"
#include <string.h>
#include <stdlib.h>

//* FUNCTIONS TO WORK DIRECTLY ON THE NODES

struct binary_tree_node *bt_node_createNode(const void *element, size_t elementSize)
{
    struct binary_tree_node *node = (struct binary_tree_node *)malloc(sizeof(struct binary_tree_node));
    if (!node)
        return NULL;

    node->bt_node_element = malloc(elementSize);
    if (!(node->bt_node_element))
    {
        free(node);
        return NULL;
    }

    memmove(node->bt_node_element, element, elementSize);

    node->bt_node_right = node->bt_node_left = NULL;

    return node;
}

struct binary_tree_node *bt_node_insert(struct binary_tree_node **root, const void *element, size_t elementSize, int(compare_function)(const void *, const void *))
{
    if (*root == NULL)
    {
        return *root = bt_node_createNode(element, elementSize);
    }

    if (compare_function(element, (*root)->bt_node_element) < 0)
    {
        return bt_node_insert(&((*root)->bt_node_left), element, elementSize, compare_function);
    }
    else
    {
        return bt_node_insert(&((*root)->bt_node_right), element, elementSize, compare_function);
    }
}

struct binary_tree_node *bt_node_findMin(struct binary_tree_node *root)
{
    struct binary_tree_node *current = root;
    while (current && current->bt_node_left)
    {
        current = current->bt_node_left;
    }
    return current;
}

struct binary_tree_node *bt_node_delete(struct binary_tree_node *root, const void *element, size_t elementSize, int (*compare_function)(const void *, const void *))
{
    if (root == NULL)
        return root;

    if (compare_function(element, root->bt_node_element) < 0)
        root->bt_node_left = bt_node_delete(root->bt_node_left, element, elementSize, compare_function);
    else if (compare_function(element, root->bt_node_element) > 0)
        root->bt_node_right = bt_node_delete(root->bt_node_right, element, elementSize, compare_function);

    else
    {
        if (root->bt_node_left == NULL)
        {
            struct binary_tree_node *temp = root->bt_node_right;
            if (root->bt_node_element)
                free(root->bt_node_element);
            free(root);
            return temp;
        }

        if (root->bt_node_right == NULL)
        {
            struct binary_tree_node *temp = root->bt_node_left;
            if (root->bt_node_element)
                free(root->bt_node_element);
            free(root);
            return temp;
        }

        struct binary_tree_node *temp = bt_node_findMin(root->bt_node_right);

        memcpy(root->bt_node_element, temp->bt_node_element, elementSize);

        root->bt_node_right = bt_node_delete(root->bt_node_right, temp->bt_node_element, elementSize, compare_function);
    }

    return root;
}

struct binary_tree_node *bt_node_search(struct binary_tree_node *root, const void *element, size_t elementSize, int (*compare_function)(const void *, const void *))
{
    if (root == NULL)
        return NULL;

    if (compare_function(element, root->bt_node_element) < 0)
        return bt_node_search(root->bt_node_left, element, elementSize, compare_function);
    else if (compare_function(element, root->bt_node_element) > 0)
        return bt_node_search(root->bt_node_right, element, elementSize, compare_function);

    return root;
}

void bt_node_visitInOrder(struct binary_tree_node *root, void (*visualizeElement)(const void *))
{
    if (root == NULL)
        return;
    bt_node_visitInOrder(root->bt_node_left, visualizeElement);
    visualizeElement(root->bt_node_element);
    bt_node_visitInOrder(root->bt_node_right, visualizeElement);
}

void bt_node_freeTree(struct binary_tree_node *root)
{
    if (root)
    {
        bt_node_freeTree(root->bt_node_left);
        bt_node_freeTree(root->bt_node_right);
        if (root->bt_node_element)
            free(root->bt_node_element);
        free(root);
    }
}

//* FUNCTION TO WORK USING THE OBJECT BINARY_TREE

struct binary_tree bt_create(size_t elementSize, int (*compare_function)(const void *, const void *))
{
    return (struct binary_tree){NULL, elementSize, compare_function};
}

int bt_insert(struct binary_tree *object, const void *element)
{
    if (bt_node_insert(&(object->bt_root), element, object->bt_elementSize, object->bt_compareFunction) == NULL)
        return FAILURE;
    return SUCCESS;
}

void bt_delete(struct binary_tree *object, const void *element)
{
    object->bt_root = bt_node_delete(object->bt_root, element, object->bt_elementSize, object->bt_compareFunction);
}

void *bt_search(struct binary_tree *object, const void *element)
{
    struct binary_tree_node *temp = bt_node_search(object->bt_root, element, object->bt_elementSize, object->bt_compareFunction);
    if (temp)
        return temp->bt_node_element;
    return NULL;
}

void bt_visitInOrder(struct binary_tree *object, void (*visualizeElement)(const void *))
{
    bt_node_visitInOrder(object->bt_root, visualizeElement);
}

void bt_freeTree(struct binary_tree *object)
{
    bt_node_freeTree(object->bt_root);
    object->bt_root = NULL;
}
