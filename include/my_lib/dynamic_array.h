#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stddef.h>
#include <errno.h>

#ifndef FAILURE
#define FAILURE -1
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

/**
 * Structure representing a dynamic array, capable of resizing and managing elements of any data type.
 *
 * @param da_ptrArray Pointer to the dynamically allocated array of elements.
 * @param da_stateVector Pointer to a dynamically allocated array where every bit represents the state
 *                       (empty (0) or occupied (1)) of each element in the dynamic array.
 * @param da_inserted The number of elements currently inserted in the dynamic array.
 * @param da_arrayCapacity The total number of elements that the dynamic array can currently hold.
 * @param da_elementSize The size, in bytes, of each element in the array, used to calculate the correct memory
 *                       offset for element access and manipulation.
 *
 * @note This structure allows for flexible management of arrays including dynamic resizing and state tracking
 *       of individual elements.
 */
struct dynamic_array
{
    void *da_ptrArray;
    unsigned char *da_stateVector;
    size_t da_inserted, da_arrayCapacity;
    size_t da_elementSize;
};

/**
 * Initializes a dynamic array with specified element size and capacity.
 * @return A dynamic_array structure with allocated memory for elements and state tracking.
 * @warning Ensure you handle memory allocation failures in your application, especially checking
 *          the return value's `da_ptrArray` and `da_stateVector` for NULL. You can check one of them
 *          since in case of failure the functions returns them both to NULL.
 */
struct dynamic_array da_create(size_t elementSize, size_t arrayCapacity);

/**
 * @return Pointer to the requested element in the array.
 * @note index is guaranteed to be within the bounds of the array using the same logic as Javascript.
 */
void *da_at(struct dynamic_array *object, int index);

/**
 * Copies an element from the dynamic array into a provided buffer.
 * @note index is guaranteed to be within the bounds of the array using the same logic as Javascript.
 * @warning Ensure the provided buffer has adequate space for the element size defined in the dynamic
 *          array to prevent buffer overflow and potential memory corruption.
 */
void da_get(struct dynamic_array *object, int index, void *buffer);

/**
 * Checks if a cell in the dynamic array is empty.
 * @return Non-zero if the cell is empty, zero otherwise.
 * @note index is guaranteed to be within the bounds of the array using the same logic as Javascript.
 */
int da_isCellEmpty(struct dynamic_array *object, int index);

/**
 * Sets the value of an element in the dynamic array.
 * @warning Ensure the provided pointer value points to a memory that is the same size as `da_elementSize`
 * @note index is guaranteed to be within the bounds of the array using the same logic as Javascript.
 */
void da_set(struct dynamic_array *object, int index, const void *value);

/**
 * Clears the contents of an element in the dynamic array and marks it as empty.
 * @note index is guaranteed to be within the bounds of the array using the same logic as Javascript.
 */
void da_clean(struct dynamic_array *object, int index);

/**
 * Updates the capacity of the dynamic array, reallocating memory as necessary.
 * @return SUCCESS on successful update, FAILURE otherwise.
 * @warning Reducing array capacity may lead to loss of data.
 * @note in case of failure, the old state of the dynamic array is mantained.
 */
int da_updateCapacity(struct dynamic_array *object, size_t newArrayCapacity);

/**
 * Frees all memory allocated for the dynamic array and resets its properties.
 * @warning Ensure to call this function to avoid memory leaks.
 */
void da_destroy(struct dynamic_array *object);

/**
 * @brief Appends an element to the dynamic array.
 *
 * This function attempts to append a new element to the end of the dynamic array. If the array is already full,
 * it tries to increase the capacity of the array by 10 elements. If the capacity update fails, the function returns FAILURE.
 * Otherwise, it inserts the new element at the end of the array and returns SUCCESS.
 *
 * @param dynArr Pointer to the dynamic array structure.
 * @param element Pointer to the element to be appended.
 * @return int SUCCESS (0) if the operation is successful, FAILURE (-1) otherwise.
 * @warning From great power comes great responsibility. The caller must ensure that the dynamic array and the element pointers are valid.
 */
int da_append(struct dynamic_array *dynArr, const void *element);

#endif