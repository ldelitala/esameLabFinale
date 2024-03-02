#include "../../include/my_lib/static_fifo.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

struct static_fifo fifost_create(size_t fifo_length, size_t element_size)
{
    struct static_fifo to_initialize = {-1, -1, fifo_length, malloc(sizeof(struct dynamic_array))};

    if (to_initialize.fifost_queue)
    {
        *(to_initialize.fifost_queue) = da_create(element_size, fifo_length);

        // check if da_create worked correctly
        if (!((to_initialize.fifost_queue)->da_ptrArray))
        {
            free(to_initialize.fifost_queue);
            to_initialize.fifost_queue = NULL;
        }
    }

    return to_initialize;
}

int fifost_isEmpty(struct static_fifo *object)
{
    return object->fifost_first_element == -1;
}

int fifost_isFull(struct static_fifo *object)
{
    return (object->fifost_first_element + 1) % object->fifost_length == object->fifost_last_element;
}

int fifost_count(struct static_fifo *object)
{
    // this formula works, just trust me
    return (object->fifost_length + object->fifost_first_element - object->fifost_last_element + 1) % object->fifost_length;
}

int fifost_enqueue(struct static_fifo *object, const void *element)
{
    if (fifost_isFull(object))
        return 1;

    (object->fifost_first_element) = ((object->fifost_first_element) + 1) % object->fifost_length;

    da_set(object->fifost_queue, object->fifost_first_element, element);

    if (object->fifost_last_element == -1)
        object->fifost_last_element = 0;

    return 0;
}

int fifost_dequeue(struct static_fifo *object, void *buffer)
{
    if (fifost_isEmpty(object))
        return 1;

    da_get(object->fifost_queue, object->fifost_last_element, buffer);

    if (object->fifost_last_element == object->fifost_first_element)
        object->fifost_last_element = object->fifost_first_element = -1;
    else
        object->fifost_last_element = ((object->fifost_last_element) + 1) % object->fifost_length;

    return 0;
}

void *fifost_peek(struct static_fifo *object)
{
    if (fifost_isEmpty(object))
        return NULL;
    return da_at(object->fifost_queue, object->fifost_last_element);
}

void fifost_clean(struct static_fifo *object)
{
    object->fifost_first_element = object->fifost_last_element = -1;
}

void fifost_destroy(struct static_fifo *object)
{
    if (object->fifost_queue)
    {
        da_destroy(object->fifost_queue);
        free(object->fifost_queue);
        object->fifost_queue = NULL;
    }
    object->fifost_length = object->fifost_first_element = object->fifost_last_element = 0;
}