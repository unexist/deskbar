#include <stdio.h>
#include <string.h>

#include "htable.h"
#include "log.h"
#include "string.h"


DbHtable *
db_htable_new (void)
{
	DbHtable *table = NULL;
	
	table = (DbHtable *) malloc (sizeof (DbHtable));

	if (table)
		{
			table->elements = 0;
			table->first		= NULL;
			table->last			= NULL;

			return (table);
		}
	
	db_log_err ("Unable to alloc memory\n");

	return (NULL);
}

void
db_htable_push (DbHtable *table,
	void *key,
	void *value)
{
	DbHtableElement *element = NULL;

	element = (DbHtableElement *) malloc (sizeof (DbHtableElement));

	if (element)
		{
			element->hash		= db_strings_hash ((char *) key);
			element->value	= value;
			element->prev		= NULL;
			element->next		= NULL;
			
			if (!table->first)
				{
					table->first	= element;
					table->last		= element;
				}
			else
				{
					table->last->next	= element;
					element->prev			= table->last;
					table->last				= element;
				}
				
			table->elements++;

			return;
		}
	
	db_log_err ("Unable to alloc memory\n");
}

static void
remove_element (DbHtable *table,
	DbHtableElement *element)
{
	DbHtableElement *cur	= NULL,
									*next	= NULL;

	cur = table->first;

	while (cur)
		{
			next = cur->next;

			if (element == cur)
				{
					if (table->first == table->last)
						{
							table->first = NULL;
							table->last	= NULL;
						}
					else if (!cur->prev)
						{
							table->first		= cur->next;
							cur->next->prev	= NULL;
						}
					else if (!cur->next)
						{
							table->last 		= cur->prev;
							cur->prev->next	= NULL;
						}
					else if (cur->prev && cur->next)
						{
							cur->prev->next = cur->next;
							cur->next->prev	= cur->prev;
						}

					free (cur);
					
					table->elements--;

					return;
				}

			cur = next;
		}
}


void *
db_htable_pop (DbHtable *table,
	void *key)
{
	unsigned int hash;

	void *ret = NULL;
	
	DbHtableElement *cur	= NULL,
			  				  *next	= NULL;

	hash = db_strings_hash ((char *) key);
	
	cur = table->first;
	
	while (cur)
		{
			next = cur->next;

			if (hash == cur->hash)
				return (cur->value);
			
			cur = next;
		}

	return (NULL);
}

void 
db_htable_destroy (DbHtable *table)
{
	DbHtableElement *cur	= NULL,
			  				  *next	= NULL;
					
	cur = table->first;

	while (cur)
		{
			next = cur->next;

			free (cur);
			
			cur = next;
			
			table->elements--;
		}

	free (table);
}
