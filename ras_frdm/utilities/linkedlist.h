/*
 * linkedlist.h
 *
 *  Created on: Jun 10, 2017
 *      Author: gonzalopelos
 */

#ifndef UTILITIES_LINKEDLIST_H_
#define UTILITIES_LINKEDLIST_H_

#include <stdint.h>
#include "mbed.h"

namespace utilities {

/**
 *  @struct node
 *  @brief The Linked List structure
 */
struct node
{
    void *data;         /*!< pointer to list member data */
    struct node *next;  /*!< pointer to the next list member */
};

/** Example using the LinkedList Class
 * @code
 *  #include "mbed.h"
 *  #include "LinkedList.h"
 *
 *  LinkedList<node>list;
 *
 *  int main()
 *  {
 *      node *tmp;
 *
 *      list.push((char *)"Two\n");
 *      list.append((char *)"Three\n");
 *      list.append((char *)"Four\n");
 *      list.push((char*)"One\n");
 *      list.append((char*)"Five\n");
 *
 *      for(int i=1; i<=list.length(); i++)
 *      {
 *          tmp = list.pop(i);
 *          printf("%s", (char *)tmp->data);
 *      }
 *
 *      error("done\n");
 *  }
 * @endcode
 */

/**
 *  @class LinkedList
 *  @brief API abstraction for a Linked List
 */
template<class retT>
class linked_list {

protected:
    retT *_head;

public:
    /** Create the LinkedList object
     */
    linked_list() {
        // clear the member
        _head = 0;

        return;
    }

    /** Deconstructor for the LinkedList object
     *  Removes any members
     */
    ~linked_list() {
        // free any memory that is on the heap
        while(remove(1) != NULL);

        return;
    }

    /** Add a member to the begining of the list
     *  @param data - Some data type that is added to the list
     *  @return The member that was just inserted (NULL if empty)
     */
    retT *push(void *data)
    {
        retT *new_node = new retT [1];
        // make sure the new object was allocated
        if (0 == new_node)
        {
            error("Memory allocation failed\n");
        }
        // update the next item in the list to the current head
        new_node->next = _head;
        // store the address to the linked datatype
        new_node->data = data;
        _head = new_node;

        return _head;
    }

    /** Add a member to the end of the list
     *  @param data - Some data type that is added to the list
     *  @return The member that was just inserted (NULL if empty)
     */
    retT *append(void *data)
    {
        retT *current = _head;
        retT *new_node = new retT [1];
        // make sure the new object was allocated
        if (0 == new_node)
        {
            error("Memory allocation failed\n");
        }
        // store the address to the linked datatype
        new_node->data = data;
        // clear the next pointer
        new_node->next = 0;
        // check for an empty list
        if (0 == current)
        {
            _head = new_node;
            return _head;
        }
        else
        {
            // look for the end of the list
            while (current->next != 0)
            {
                current = current->next;
            }
            // and then add the new item to the list
            current->next = new_node;
        }

        return current->next;
    }

    /** Remove a member from the list
     *  @param loc - The location of the member to remove
     *  @return The head of the list
     */
    retT *remove(uint32_t loc)
    {
        retT *current = _head;
        retT *prev = 0;
        // make sure we have an item to remove
        if ((loc <= length()) && (loc > 0))
        {
            // move to the item we want to delete
            if (1 == loc)
            {
                _head = current->next;
                delete [] current;
            }
            else
            {
                for (uint32_t i=2; i<=loc; ++i)
                {
                    prev = current;
                    current = current->next;
                }
                // store the item + 1 to replace what we are deleting
                prev->next = current->next;
                delete [] current;
            }
        }

        return _head;
    }

    /** Get access to a member from the list
     *  @param loc - The location of the member to access
     *  @return The member that was just requested (NULL if empty or out of bounds)
     */
    retT *pop(uint32_t loc)
    {
        retT *current = _head;
        // make sure we have something in the location
        if ((loc > length()) || (loc == 0))
        {
            return 0;
        }
        // and if so jump down the list
        for (uint32_t i=2; i<=loc; ++i)
        {
            current = current->next;
        }

        return current;
    }

    /** Get the length of the list
     *  @return The number of members in the list
     */
    uint32_t length(void)
    {
        int32_t count = 0;
        retT *current = _head;
        //loop until the end of the list is found
        while (current != 0)
        {
            ++count;
            current = current->next;
        }

        return count;
    }
};

} /* namespace utilities */

#endif /* UTILITIES_LINKEDLIST_H_ */
