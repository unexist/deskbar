#ifndef DB_HTABLE_H
#define DB_HTABLE_H 1

typedef struct dbhtableelement_t
{
	unsigned int hash;
	
	void *value;
	
	struct dbhtableelement_t *prev;
	struct dbhtableelement_t *next;
} DbHtableElement; 

typedef struct dbhtable_t
{
	int elements;

	struct dbhtableelement_t *first;
	struct dbhtableelement_t *last;
} DbHtable;

DbHtable *db_htable_new (void);

void db_htable_push (DbHtable *table, void *key, void *value);
void *db_htable_pop (DbHtable *table, void *key);
void db_htable_destroy (DbHtable *table);

#endif /* DB_HTABLE_H */
