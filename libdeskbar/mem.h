#ifndef DB_MEM_H
#define DB_MEM_H 1

#define db_mem_alloc(size) \
	db_mem_private_alloc ((void *) plug, size);

#define db_mem_realloc(addr,newsize) \
	db_mem_private_realloc ((void *) plug, (void *) addr, newsize);

#define db_mem_free(addr) \
	db_mem_private_free ((void *) plug, (void *) addr);

void db_mem_table_new (void);

void *db_mem_private_alloc (void *id, int size);
void *db_mem_private_realloc (void *id, void *addr, int newsize);
void db_mem_private_free (void *id, void *addr);

void db_mem_pool_destroy (void *id);

void db_mem_table_destroy (void);

void db_mem_toggle_profile (void);
void db_mem_profile (void);

#endif /* DB_MEM_H */
