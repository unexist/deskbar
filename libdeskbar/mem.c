#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "mem.h"
#include "plug.h"

typedef struct dbmemchunk_t
{
  int size;

  void *addr;
} DbMemChunk;

typedef struct dbmempool_t
{
  int n;

  int allocs;
  int frees;

  struct dbmemchunk_t *mc;
} DbMemPool;

typedef struct dbmemtable_t
{
	void *id;

	DbMemPool *mp;
} DbMemTable;

static DbMemTable *mt = NULL;

static int n = 0;
static int profile = 0;

 /** get_pool_by_id:
  * @id: Pointer to the internal plugin data.
	*
	* Returns the memory pool of the given id or NULL if not found.
	**/

static DbMemPool *
get_pool_by_id (void *id)
{
	int i;

	for (i = 0; i < n; i++)
		if (mt[i].id == id)
			return (mt[i].mp);
	
	return (NULL);
}

 /** mem_pool_new:
  * @id: Pointer to the internal plugin data.
	*
	* Creates a new memory pool for the given id and stores it into
	* the memory table. Aborts on failure.
	**/

static DbMemPool *
mem_pool_new (void *id)
{
	DbMemPool *mp = NULL;
	
	mp = (DbMemPool *) malloc (sizeof (DbMemPool));

	if (!mp)
		{
			db_log_err ("Unable to alloc memory!\n");

			abort ();
		}

	mp->n				= 0;
	mp->allocs	= 0;
	mp->frees		= 0;
	mp->mc			= NULL;

	mt = (DbMemTable *) realloc (mt, (sizeof (DbMemTable) * (n + 1)));

	mt[n].id = id;
	mt[n].mp = mp;

	n++;

	return (mp);
}

 /** db_mem_table_new:
  * 
	* Creates the memory table and aborts on failure.
	**/

void
db_mem_table_new (void)
{
	mt = (DbMemTable *) malloc (sizeof (DbMemTable));
	
	if (!mt)
		{
			db_log_err ("Unable to alloc memory!\n");

			abort ();
		}

	db_log_mesg ("Created memory table\n");
}

 /** db_mem_private_alloc:
  * @id: Pointer to the internal plugin data.
	* @size: Size of the needed memory.
	*
	* Checks if a memory pool exists and returns a pointer to
	* the newly allocated memory. Otherweise the pool will be 
	* created first.
	* Hint: For internal use only, use the macros instead!
	**/

void *
db_mem_private_alloc (void *id,
	int size)
{
	DbMemPool *mp = NULL;
	
	mp = get_pool_by_id (id);

	if (!mp)
		{
			/* Create memory table if it does not exist */
			mp = mem_pool_new (id);

			if (!mp)
				{
					db_log_err ("Can't get address of memory pool? Oh no!\n");

					abort ();
				}
		}
		
	mp->mc = (DbMemChunk *) realloc (mp->mc, (sizeof (DbMemChunk) * (mp->n + 1)));

	mp->mc[mp->n].size = size;
	mp->mc[mp->n].addr = (void *) malloc (size);

	if (!mp->mc || !mp->mc[mp->n].addr)
		{
			db_log_err ("Unable to alloc memory!\n");

			abort ();
		}
		
	mp->n++;
	mp->allocs++;

	db_log_debug ("Plugin %s allocated %d bytes\n", ((DbPlug *) id)->name, size);

	return (mp->mc[mp->n - 1].addr);
}

void *
db_mem_private_realloc (void *id,
	void *addr,
	int newsize)
{
	int i;
	
	DbMemPool *mp = NULL;
	
	mp = get_pool_by_id (id);

	if ((!mp) || (!addr))
		return (db_mem_private_alloc (id, newsize));
		
	for (i = 0; i < mp->n; i++)
		if (mp->mc[i].addr == addr)
			{
				mp->mc[i].size	= newsize;
				mp->mc[i].addr	= (void *) realloc (addr, newsize);
				
				mp->allocs++;

				db_log_debug ("Plugin %s re-allocated %d bytes\n", 
					((DbPlug *) id)->name, newsize);

				break;
			}
	
	return (mp->mc[i].addr);
}

void
db_mem_private_free (void *id,
	void *addr)
{
	int i, j;
	int size;
	
	DbMemPool *mp = NULL;
	
	mp = get_pool_by_id (id);

	if (!mp)
		{
			db_log_err ("Can't get address of memory pool? Oh no!\n");

			abort ();
		}

	for (i = 0; i < mp->n; i++)
		if (mp->mc[i].addr == addr)
			{
				size = mp->mc[i].size;

				free (mp->mc[i].addr);

				for (j = (i + 1); j < mp->n; j++)
					{
						mp->mc[j - 1].size = mp->mc[j].size;
						mp->mc[j - 1].addr = mp->mc[j].addr;
					}

				mp->n--;
				mp->frees++;

				mp->mc = (DbMemChunk *) realloc (mp->mc, (sizeof (DbMemChunk) * (mp->n + 1)));

				db_log_debug ("Plugin %s free'd %d bytes\n", ((DbPlug *) id)->name, size);

				break;
			}
}

void
db_mem_pool_destroy (void *id)
{
	int i, j;
	int size = 0;
	
	DbMemPool *mp = NULL;
	
	mp = get_pool_by_id (id);

	if (!mp)
		return;

	for (i = 0; i < n; i++)
		if (mt[i].mp == mp)
			if (mp->n > 0)
				{
					for (j = 0; j < mp->n; j++)
						{
							size += mp->mc[j].size;
					
							free (mp->mc[j].addr);
						}

					free (mp->mc);

					db_log_mesg ("Leak summary: Allocs %d, Frees %d, Rescued %d bytes\n", 
												mp->allocs, mp->frees, size);
														
					for (j = (i + 1); j < n; j++)
						{
							mt[j - 1].id = mt[j].id;
							mt[j - 1].mp = mt[j].mp;
						}

					mt = (DbMemTable *) realloc (mt, (sizeof (DbMemTable) * (n + 1)));

					free (mp);
					n--;

				}
}

void
db_mem_table_destroy (void)
{
	db_log_mesg ("Destroyed memory table\n");
}

void
db_mem_toggle_profile (void)
{
	profile = !profile;
}

void
db_mem_profile (void)
{
	int i, j;
	int size = 0;

	if ((profile == 0) || (n == 0))
		return;
		
	db_log_mesg ("Memory in use at exit:\n");
	db_log_mesg ("    Plugin  |  Allocs  |   Frees   |  Bytes\n");
	db_log_mesg ("----------------------------------------------\n");

	for (i = 0; i < n; i++)
		{
			if (mt[i].mp->n > 0)
				for (j = 0; j < mt[i].mp->n; j++)
					size += mt[i].mp->mc[j].size;
					
			db_log_mesg ("  %8s  |  %6d  |   %5d   |  %5d\n", ((DbPlug *) mt[i].id)->name, 
								   mt[i].mp->allocs, mt[i].mp->frees, size);
		}
}
