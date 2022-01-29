#ifndef _ITER_H
#define _ITER_H

typedef struct Iter {
	void *node;
	unsigned int signature;
} Iter;

Iter 	*iter_new();
void	*iter_next(Iter *iter);
void	iter_end(Iter **iter);


#endif
