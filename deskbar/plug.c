#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <setjmp.h>

#include "plug.h"
#include "sig.h"

#include "libdeskbar/list.h"
#include "libdeskbar/log.h"
#include "libdeskbar/mem.h"
#include "libdeskbar/htable.h"
#include "libdeskbar/times.h"
#include "libdeskbar/strings.h"

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

static int interval = 0;

static DbList *pluglist	= NULL;

static int
safe_call (DbPlugElement *element,
	DbPlugFunc plugfunc)
{
	if (plugfunc)
		{
			if (setjmp (env) == 0)
				plugfunc (element->options);
			else
				{
					db_log_mesg ("Ayyyee! Segmentation fault in plugin %s!\n", 
											 element->data->name);
					db_plug_unload (element);

					return (1);
				}
		}
	
	return (0);
}

static int
get_gcd (int a,
	int b)
{
	int c;

	do
		{
			if (a < b)
				{
					c = a;
					a = b;
					b = c;
				}

			c = a - b;
			a = b;
			b = c;
		}
	while (c != 0);

	return (a);
}

int
db_plug_get_interval (void)
{
	return (interval);
}

void
db_plug_init (void)
{
	pluglist = db_list_new ();

	db_mem_table_new ();
}

void
db_plug_load (DbHtable *table)
{
	int update;

	char *err			= NULL;
	char *file		= NULL;
	char *check		= NULL;

	char buf[120];

	DbPlug *(*entrypoint) (void);
	
	DbPlugElement *element = NULL;

	element = (DbPlugElement *) calloc (1, sizeof (DbPlugElement));

	if (!element)
		{
			db_log_err ("Unable to alloc memory!\n");

			return;
		}

	/* Get file name */
	file = (char *) db_htable_pop (table, "name");

	if (!file)
		{
			db_log_err ("Cannot open plugin!\n");

			return;
		}

	db_strings_tolower (file);

	snprintf (buf, sizeof (buf), "%s/%s.so", PLUGIN_DIR, file);
	
	element->handle	= dlopen (buf, RTLD_LAZY);
							
	if ((err = dlerror ()))
		{
			db_log_err ("Cannot load plugin `%s'\n", file);
			db_log_debug ("dlopen (): %s\n", err);

			free (element);

			return;
		}

	/* Get entrypoint and call it */
	entrypoint		= dlsym (element->handle, "db_plug_init");
	element->data	= (*entrypoint) ();
		
	/* Validate id */
	check = db_htable_pop (table, "id");

	if (check)
		element->id = db_strings_hash (check);
	else
		element->id = db_strings_hash (file);
		
	/* Validate interval */
	check = db_htable_pop (table, "interval");

	if (check)
		element->data->interval = atoi (check);
	else
		element->data->interval = 2;
	
	if (interval == 0)
		interval = element->data->interval;
	else
		interval = get_gcd (interval, element->data->interval);
	
	/* Validate options */
	check = db_htable_pop (table, "options");

	if (check)
		element->options = strdup (check);

	db_list_insert (pluglist, (void *) element);

	db_log_mesg ("Loaded plugin %s (%ds)\n",
		element->data->name, element->data->interval);

	/* Call plugin create/update functions */
	safe_call (element, element->data->create);
	safe_call (element, element->data->update);
							
	/* Update times */
	update	= db_time_current ();
	update += (60 - (update % 60));

	element->updated = (update + element->data->interval);
}

static void
update_plug (void *data,
	void *user_data)
{
	DbPlugElement *element = NULL;

	element = (DbPlugElement *) data;

	if (db_time_update_time () >= (element->updated + db_time_update_diff ()))
		{
			if (safe_call (element, (DbPlugFunc) element->data->update))
				return;

			element->updated = (db_time_update_time () + element->data->interval);
		}
}

void
db_plug_update (void)
{
	db_time_update ();
	
	db_list_foreach (pluglist, update_plug, NULL);
}

static void
destroy_plug (void *data,
	void *user_data)
{
	DbPlugElement *element = NULL;

	element = (DbPlugElement *) data;

	if (element->data->destroy)
		(*element->data->destroy) ();
}

static void
destroy_data (void *data,
	void *func_data)
{
	DbPlugElement *element = NULL;

	element = (DbPlugElement *) data;

	db_mem_pool_destroy (element->data);

	if (element->options)
		free (element->options);

	db_log_mesg ("Unloaded plugin %s\n", element->data->name);

	if (element->handle)
		dlclose (element->handle);

	free (element);
}

void
db_plug_unload (DbPlugElement *element)
{
	db_list_remove (pluglist, (void *) element, destroy_data);
}
	
void
db_plug_unload_all (void)
{
	db_list_foreach (pluglist, destroy_plug, NULL);
	db_list_destroy (pluglist, destroy_data);
}

static int
get_plug (void *data,
	void *func_data)
{
	unsigned long id;
	
	DbPlugElement *element = NULL;
	
	element = (DbPlugElement *) data;
	id			= *(unsigned long *) func_data;

	if (element->id == id)
		return (0);
	else
		return (1);
}

void *
db_plug_get_by_name (char *name)
{
	unsigned long id;
	
	DbPlugElement *element = NULL;

	id = db_strings_hash (name);
	
	element = (DbPlugElement *) db_list_find (pluglist, get_plug, (void *) &id);

	if (element)
		return (element->data->data);
	else
		return (NULL);
}
