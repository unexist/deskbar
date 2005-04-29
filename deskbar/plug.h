#ifndef DB_PLUG_H
#define DB_PLUG_H 1

#include "libdeskbar/htable.h"

typedef struct dbplug_t
{
	char *name;
	
	void (*create) (void);
	void (*update) (void);
	void (*destroy) (void);

	char *data;
	char *format;

	int interval;	
} DbPlug;

typedef struct dbplugelement_t
{
	void *handle;

	double updated;

	struct dbplug_t *data;
} DbPlugElement;

typedef void (*DbPlugFunc) (void);

int db_plug_get_interval (void);

void db_plug_init (void);
void db_plug_load (DbHtable *table);
void db_plug_update (void);
void db_plug_destroy (void);
void db_plug_unload (DbPlugElement *element);
void db_plug_unload_all (void);

char *db_plug_get_by_name (char *name);

#endif /* DB_PLUG_H */
