#ifndef DB_DESK_H
#define DB_DESK_H 1

#include "libdeskbar/htable.h"

typedef enum dbobjtype_t
{
	DB_OBJ_TEXT,
	DB_OBJ_METER
} DbObjType;

void db_desk_init (void);
void db_desk_draw (void);
void db_desk_obj_new (DbObjType type, DbHtable *table);
void db_desk_event_loop (void);
void db_desk_destroy (void);

#endif /* DB_DESK_H */
