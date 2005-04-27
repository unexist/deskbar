#ifndef DB_LIST_H
#define DB_LIST_H 1

typedef struct dblistelement_t
{
	void *data;

	struct dblistelement_t *prev;
	struct dblistelement_t *next;
} DbListElement;

typedef struct dblist_t
{
	int elements;
	
	struct dblistelement_t *first;
	struct dblistelement_t *last;
} DbList;

typedef void (*DbListFunc) (void *, void *);
typedef int (*DbListCmpFunc) (void *, void *);

DbList *db_list_new (void);

void db_list_insert (DbList *list, void *data);
void db_list_remove (DbList *list, void *data, DbListFunc destructor);
void db_list_foreach (DbList *list, DbListFunc func, void *func_data);
void db_list_destroy (DbList *list, DbListFunc destructor);

void *db_list_find (DbList *list, DbListCmpFunc func, void *func_data);

#endif /* DB_LIST_H */
