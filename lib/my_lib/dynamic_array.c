#include "../../include/my_lib/dynamic_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! INTERNAL FUNCTIONS

//*stateVector

#ifndef OCCUPIED
#define OCCUPIED 1
#endif

#ifndef EMPTY
#define EMPTY 0
#endif

/**
 * @brief Initializes a vector to track the state of each cell as either OCCUPIED (1) or EMPTY (0).
 *
 * Allocates memory to hold the state of each cell in a bit array, where each bit represents the state of a cell.
 * The memory is initialized to 0 (EMPTY).
 *
 * @param da_arrayCapacity The capacity of the array represented by the state vector.
 * @return unsigned char* A pointer to the allocated state vector, or NULL if memory allocation fails.
 * @warning The caller must ensure that the returned pointer is freed to avoid memory leaks.
 */
unsigned char *init_stateVector(size_t da_arrayCapacity)
{
    size_t bytes = (da_arrayCapacity + 7) / 8;
    unsigned char *stateVector = (unsigned char *)malloc(bytes);

    if (stateVector)
        memset(stateVector, 0, bytes);

    return stateVector;
}

/**
 * @brief Sets the state of a specific cell in the bit array.
 *
 * @param state The new state of the cell (OCCUPIED(1) or EMPTY(0)).
 * @note The function does not check if the index is within the bounds of the array, and the caller must ensure this to prevent undefined behavior.
 */
void set_cell_state(unsigned char *stateVector, size_t index, int state)
{
    size_t byte_index = index / 8;
    size_t bit_offset = index % 8;

    if (state)
        stateVector[byte_index] |= (1 << bit_offset); // set bit to 1
    else
        stateVector[byte_index] &= ~(1 << bit_offset); // set bit to 0
}

/**
 * @brief Checks if a cell in the bit array is empty.
 *
 * @return int 1 if the cell is empty (0), or 0 if the cell is occupied (1).
 * @note The function assumes the index is within the bounds of the array. The caller is responsible for ensuring this to prevent undefined behavior.
 */
int is_cell_empty(unsigned char *stateVector, size_t index)
{
    return !(stateVector[index / 8] & (1 << (index % 8)));
}

/**
 * @brief Updates the capacity of the state vector, reallocating memory as necessary.
 *
 * Adjusts the size of the state vector to accommodate a new array capacity. Initializes new bits to EMPTY (0)
 * and maintains the state of existing bits. If the new capacity is smaller, extra bits at the end are truncated.
 *
 * @return unsigned char* A pointer to the updated state vector, or NULL if memory reallocation fails.
 * @warning The caller must handle NULL returns to prevent memory leaks and must not assume the original pointer is still valid.
 */
unsigned char *update_stateVector(unsigned char *stateVector, size_t oldArrayCapacity, size_t newArrayCapacity)
{
    size_t new_bytes = (newArrayCapacity + 7) / 8,
           old_bytes = (oldArrayCapacity + 7) / 8;

    int new_last_bit_offset = newArrayCapacity % 8;

    unsigned char *stateVectorReallocated = realloc(stateVector, new_bytes);
    if (!stateVectorReallocated)
        return NULL;

    if (newArrayCapacity > oldArrayCapacity)
        memset(stateVectorReallocated + old_bytes, 0, new_bytes - old_bytes); // imposti i nuovi byte a 0

    if (newArrayCapacity < oldArrayCapacity)
    {
        if (new_last_bit_offset == 0)
        {
            // Se newArrayCapacity è un multiplo esatto di 8, tutti i bit dell'ultimo byte sono già in uso e non c'è bisogno di applicare una maschera.
        }
        else
        {
            unsigned char mask = ~((1 << (8 - new_last_bit_offset)) - 1);
            stateVectorReallocated[new_bytes - 1] &= mask;
        }
    }

    return stateVectorReallocated;
}

/**
 * @brief Counts the number of active (OCCUPIED) bits in the state vector.
 *
 * @param da_arrayCapacity The capacity of the array represented by the state vector.
 * @return size_t The number of bits set to OCCUPIED (1).
 * @note This function assumes the state vector accurately reflects the current state of all cells.
 */
size_t count_active_bits(unsigned char *stateVector, size_t da_arrayCapacity)
{
    size_t bytes = (da_arrayCapacity + 7) / 8,
           count = 0,
           sizeArrayInt = bytes / 8,
           temp;

    __int64_t *arrayInt = (__int64_t *)stateVector;

    for (size_t i = 0; i < sizeArrayInt; i++)
    {
        temp = arrayInt[i];

        while (temp > 0)
        {
            temp = temp & (temp - 1);
            count++;
        }
    }

    unsigned char byteTemp;

    for (size_t i = 0; i < bytes % 8; i++)
    {
        byteTemp = stateVector[bytes - i - 1];

        while (byteTemp != 0)
        {
            byteTemp = byteTemp & (byteTemp - 1);
            count++;
        }
    }

    return count;
}

/**
 * @brief Calculates the real index in the array, emulating JavaScript's behavior for array indexing.
 *
 * This function adjusts the provided index to ensure it falls within the bounds of the array, similar to how JavaScript
 * handles array index access. In JavaScript, accessing an array with a negative index or an index greater than the array's length
 * wraps around the array. This function implements the same logic for circular array behavior, making sure the returned index
 * is always within the array's boundaries.
 *
 * @return size_t The adjusted index, guaranteed to be within the bounds of the array.
 */
size_t real_index(int index, size_t arrayCapacity)
{
    return (size_t)(index % arrayCapacity + arrayCapacity) % arrayCapacity;
}

//! PUBLIC FUNCTIONS

struct dynamic_array da_create(size_t elementSize, size_t arrayCapacity)
{
    struct dynamic_array result = {
        .da_ptrArray = malloc(elementSize * arrayCapacity),
        .da_stateVector = init_stateVector(arrayCapacity),
        .da_inserted = 0,
        .da_arrayCapacity = arrayCapacity,
        .da_elementSize = elementSize};

    if (result.da_ptrArray && !(result.da_stateVector))
    {
        free(result.da_ptrArray);
        result.da_ptrArray = NULL;
    }

    if (!(result.da_ptrArray) && result.da_stateVector)
    {
        free(result.da_stateVector);
        result.da_stateVector = NULL;
    }

    return result;
}

void *da_at(struct dynamic_array *object, int index)
{
    return object->da_ptrArray + real_index(index, object->da_arrayCapacity) * object->da_elementSize;
}

void da_get(struct dynamic_array *object, int index, void *buffer)
{
    void *cell_to_copy = da_at(object, index);
    memmove(buffer, cell_to_copy, object->da_elementSize);
}

int da_isCellEmpty(struct dynamic_array *object, int index)
{
    return is_cell_empty(object->da_stateVector, real_index(index, object->da_arrayCapacity));
}

void da_set(struct dynamic_array *object, int index, const void *value)
{
    size_t realIndex = real_index(index, object->da_arrayCapacity);

    if (is_cell_empty(object->da_stateVector, realIndex))
    {
        set_cell_state(object->da_stateVector, realIndex, OCCUPIED);
        (object->da_inserted)++;
    }

    memmove(da_at(object, index), value, object->da_elementSize);
}

void da_clean(struct dynamic_array *object, int index)
{
    size_t realIndex = real_index(index, object->da_arrayCapacity);

    if (!is_cell_empty(object->da_stateVector, realIndex))
    {
        memset(da_at(object, index), 0, object->da_elementSize);
        set_cell_state(object->da_stateVector, realIndex, EMPTY);
        (object->da_inserted)--;
    }
}

int da_updateCapacity(struct dynamic_array *object, size_t newArrayCapacity)
{
    size_t oldArrayCapacity = object->da_arrayCapacity;

    struct dynamic_array *ptrArrayTemp = realloc(object->da_ptrArray, newArrayCapacity * object->da_elementSize);
    unsigned char *stateVectorTemp = update_stateVector(object->da_stateVector, oldArrayCapacity, newArrayCapacity);
    if (!ptrArrayTemp || !stateVectorTemp)
        return FAILURE;

    object->da_ptrArray = ptrArrayTemp;
    object->da_stateVector = stateVectorTemp;
    object->da_arrayCapacity = newArrayCapacity;

    if (newArrayCapacity < oldArrayCapacity)
        object->da_inserted = count_active_bits(object->da_stateVector, newArrayCapacity);

    return SUCCESS;
}

void da_destroy(struct dynamic_array *object)
{
    if (object->da_ptrArray)
    {
        free(object->da_ptrArray);
        object->da_ptrArray = NULL;
    }
    if (object->da_stateVector)
    {
        free(object->da_stateVector);
        object->da_stateVector = NULL;
    }
    object->da_arrayCapacity = object->da_inserted = object->da_elementSize = 0;
}

int da_append(struct dynamic_array *dynArr, const void *element)
{
    if (dynArr->da_inserted == dynArr->da_arrayCapacity &&
        da_updateCapacity(dynArr, dynArr->da_arrayCapacity + 10) == FAILURE)

        return FAILURE;

    da_set(dynArr, dynArr->da_inserted, element);
    return SUCCESS;
}
