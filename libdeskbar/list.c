#include <stdlib.h>

#include "libdeskbar/list.h"
#include "libdeskbar/log.h"

DbList *
db_list_new (void)
{
	DbList *list = NULL;

	list = (DbList *) malloc (sizeof (DbList));

	if (list)
		{
			list->elements	= 0;
			list->first			= NULL;
			list->last			= NULL;

			return (list);
		}
	
	db_log_err ("Unable to alloc memory\n");
	
	return (NULL);
}

void
db_list_insert (DbList *list,
	void *data)
{
	DbListElement *element = NULL;

	element = (DbListElement *) malloc (sizeof (DbListElement));

	if (element)
		{
			element->data = data;
			element->prev = NULL;
			element->next = NULL;
			
			if (!list->first)
				{
					list->first = element;
					list->last	= element;
				}
			else
				{
					list->last->next	= element;
					element->prev			= list->last;
					list->last				= element;
				}
				
			list->elements++;

			return;
		}
	
	db_log_err ("Unable to alloc memory\n");
}

void
db_list_remove (DbList *list,
	void *data,
	DbListFunc destructor)
{
	DbListElement *element	= NULL,
								*cur			= NULL,
						    *next			= NULL;

	element = (DbListElement *) data;
					
	cur = list->first;

	while (cur)
		{
			next = cur->next;

			if (element == cur->data)
				{
					if (list->first == list->last)
						{
							list->first = NULL;
							list->last	= NULL;
						}
					else if (!cur->prev)
						{
							list->first			= cur->next;
							cur->next->prev	= NULL;
						}
					else if (!cur->next)
						{
							list->last 			= cur->prev;
							cur->prev->next	= NULL;
						}
					else if (cur->prev && cur->next)
						{
							cur->prev->next = cur->next;
							cur->next->prev	= cur->prev;
						}

					(*destructor) ((void *) cur->data, NULL);

					free (cur);
					
					list->elements--;

					return;
				}

			cur = next;
		}
	
}

void
db_list_foreach (DbList *list,
	DbListFunc func,
	void *func_data)
{
	DbListElement *cur	= NULL,
			  			  *next	= NULL;
					
	cur = list->first;

	while (cur)
		{
			next = cur->next;

			(*func) ((void *) cur->data, (void *) func_data);

			cur = next;
		}
}

void
db_list_destroy (DbList *list,
	DbListFunc destructor)
{
	DbListElement *cur	= NULL,
			  			  *next	= NULL;
					
	cur = list->first;

	while (cur)
		{
			next = cur->next;

			(*destructor) ((void *) cur->data, NULL);

			free (cur);
			
			cur = next;
			
			list->elements--;
		}

	free (list);
}

void *
db_list_find (DbList *list,
	DbListCmpFunc func,
	void *func_data)
{
	DbListElement *cur	= NULL,
			  			  *next	= NULL;
					
	cur = list->first;

	while (cur)
		{
			next = cur->next;

			if (!(*func) ((void *) cur->data, (void *) func_data))
				return ((void *) cur->data);

			cur = next;
		}
	
	return (NULL);
}
