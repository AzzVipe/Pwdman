#ifndef __LIST_H
#define __LIST_H 

/**
 * ----------------------------------------------------------------------------
 * List Abstract Data Type 
 * Author : Shek Muktar 				Date: 24-11-2018
 *
 * List headers
 * Contains abstract data type for list
 * and function protoype declaration
 * ----------------------------------------------------------------------------
 *
 * A doubly linked list of list ADT implementation
 *
 * Usage:
 * o Create new list
 *   List *list = list_new(sizeof(double), NULL)
 *     sizeof(double) is for what element you want to put in the list
 *
 * o Push data at the end of list
 *   double data = 3.45D;
 *   list_pushback(list, &data)
 *
 * o Pop data from list
 *   void *popped = list_popback(list)
 *     using popped data
 *       printf("Popped: %lf\n", *(double *)popped)
 *     then free it
 *       free(popped);
 *
 * o Get top of the list
 *   void *data = list_front(list)
 *     use the data
 *     NOTE: the address returned is the original data in the list
 *     that you pushed, therfore any changes in data will change the orig data
 *
 * o Destroy the entire list nodes and list it self
 *   int count = list_destroy(&list)
 *   count is returned this is total number of elements that is availabe
 *   before list_destroy() function call
 *
 * o Reverse the list (front is back and vice vers, i.e. recent push_back() becomes first in list)
 *   list_reverse(list)
 *   no return value
 *
 * NOTE: Since it is a linked list you can push element in front and back
 * of the list by calling list_pushfront() and list_pushback() respectively.
 *
 * All functions:
 *  List *list_new(size_t data_size, void (*destroyer)(void *))
 *  void *list_pushfront(List *ls, void *data)
 *  void *list_pushback(List *ls, void *data)
 *  void *list_push(List *ls, void *data) 		* same as list_pushback() 			*
 *  void *list_popfront(List *ls)
 *  void *list_popback(List *ls)
 *  void *list_pop(List *ls)					* same as list_popback()  			*
 *  void *list_front(List *ls)					* returns first element of the list *
 *  void *list_back(List *ls)					* returns last element of the list  *
 *  int  list_destroy(List **ls)
 *  void *list_search(List *ls, void *needle, int (*search_callback)(void *data, void *needle))
 *  int   list_remove(List *ls, void *needle, int (*search_callback)(void *data, void *needle));
 */

typedef struct {
	size_t size;
	size_t elem_size;
	void   *head;
	void   *tail;
	void (*destroyer)(void *);
} List;

/**
 * Creates new list object
 *
 * @param size_t elem_size  size of the data to be pushed in the list
 * @param void (*destroyer)(void *)	pointer to user destroyer function
 * 									responsible for freeing user data
 * @return Stack*  list object address
 */
List *list_new(size_t elem_size, void (*destroyer)(void *));

/**
 * Push front user data into list 
 *
 * @param  Stack*  list object address returned by list_new()
 * @param  void*   address of user data to be pushed
 * @return void*   address of the data that is pushed (for chaining)
 */
void  *list_pushfront(List *ls, void *data);

/**
 * Push back user data into list 
 *
 * @param  Stack*  list object address returned by list_new()
 * @param  void*   address of user data to be pushed
 * @return void*   address of the data that is pushed (for chaining)
 */
void  *list_pushback(List *ls, void *data);

/**
 * Push back user data into list (alias for push_back() function)
 *
 * @param  Stack*  list object address returned by list_new()
 * @param  void*   address of user data to be pushed
 * @return void*   address of the data that is pushed (for chaining)
 */
void  *list_push(List *ls, void *data);

/**
 * Pop front (first) element of the list
 *
 * @param  Stack*  list object address returned by list_new()
 * @return void*   address of the user data
 * 				   (after use user must free it by himself)
 */
void  *list_popfront(List *ls);

/**
 * Pop back (last) element from the the list
 *
 * @param  Stack*  list object address returned by list_new()
 * @return void*   address of the user data
 * 				   (after use user must free it by himself)
 */
void  *list_popback(List *ls);

/**
 * Pop back (last) element from the the list (alias for pop_back() function)
 *
 * @param  Stack*  list object address returned by list_new()
 * @return void*   address of the user data
 * 				   (after use user must free it by himself)
 */
void  *list_pop(List *ls);

/**
 * Returns the address of the original user data from front of list
 * Note: modifying this data referencially will modify the original
 *
 * @param  Stack*  list object address returned by list_new()
 * @return void*   address of the user data
 * 				   (after use user must free it by himself)
 */
void  *list_front(List *ls);

/**
 * Returns the address of the original user data from back of list
 * Note: modifying this data referencially will modify the original
 *
 * @param  Stack*  list object address returned by list_new()
 * @return void*   address of the user data
 * 				   (after use user must free it by himself)
 */
void  *list_back(List *ls);

/**
 * Searches a particular node based on the search callback function
 * passed as second argument
 * argument to callback function will be a pointer to user data
 * Note: modifying this data referencially will modify the original
 *
 * @param  Stack*  list object address returned by list_new()
 * @return void*   pointer to user data if match found, else NULL
 */
void  *list_search(List *ls, void *needle, int (*search_callback)(void *data, void *needle));

/**
 * Deletes all the list node and user data that matches given needle 
 *
 * @param  List*  list object address returned by list_new()
 * @param  void*  needle that to be deleted from the list
 * @param  int (*search_callback)()  search callback function that will
 * 									 match the needle with user data
 * @return int    1 if delete was successful else 0 (zero) 
 */
int   list_remove(List *ls, void *needle, int (*search_callback)(void *data, void *needle));

/**
 * Destroys entire list (Nodes of the list and list itself)
 * Note: after destroying list don't use the list object
 *       because it is nulled. You can create new one and use it
 *
 * @param  Stack**  address of the list object (pntr to pntr to Stack)
 * @return int      total number of destroyed nodes
 */
int   list_destroy(List **ls);

/**
 * Reverse the list (upside down, i.e. recent push becomes last in list)
 *
 * @param  Stack*  list object address returned by list_new()
 * @return void
 */
void  list_reverse(List *ls);

int   list_isempty(List *ls);
List *list_clone(List *ls);
void  *list_getiter(List *ls);

#endif