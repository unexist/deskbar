#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <setjmp.h>

#include "plug.h"
#include "sig.h"

#include "libdeskbar/list.h"
#include "libdeskbar/log.h"
#include "libdeskbar/mem.h"
#include "libdeskbar/htable.h"
#include "libdeskbar/times.h"

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

static int interval = 0;

static DbList *pluglist			= NULL;
static DbHtable *plugtable	= NULL;

static int
save_call (DbPlugElement *element,
	DbPlugFunc plugfunc,
	const char *name)
{
	if (plugfunc)
		{
			if (setjmp (env) == 0)
				plugfunc ();
			else
				{
					db_log_mesg ("Ayyyee! Segmentation fault in plugin %s!\n", 
											 element->data->name);
					db_log_debug ("Call to %s () failed\n", name);
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
	pluglist	= db_list_new ();

	db_mem_table_new ();
}

void
db_plug_load (const char *file,
	const char *format,
	int plug_interval)
{
	char *err	= NULL;

	char buf[120];

	DbPlug *(*entrypoint) (void);
	
	DbPlugElement *element = NULL;

	element = (DbPlugElement *) malloc (sizeof (DbPlugElement));

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
		
	/* Setup plugin */
	element->data->interval = plug_interval;

	if (format)
		element->data->format = strdup (format);

	db_list_insert (pluglist, (void *) element);

	db_log_mesg ("Loaded plugin %s (%ds)\n",
		element->data->name, element->data->interval);

	/* Call plugin create/update functions */
	save_call (element, element->data->create, "create");
	save_call (element, element->data->update, "update");
							
	/* Update times */
	element->updated = (db_time_current () + element->data->interval);
		
	/* Update internal time */
	if (interval == 0)
		interval = element->data->interval;
	else
		interval = get_gcd (interval, element->data->interval);
}

static void
update_plug (void *data,
	void *user_data)
{
	DbPlugElement *element = NULL;

	element = (DbPlugElement *) data;

	if (db_time_update_time () >= (element->updated + db_time_update_diff ()))
		{
			if (save_call (element, (DbPlugFunc) element->data->update, "update"))
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

	if (element->data->format)
		free (element->data->format);

	db_log_mesg ("Unloaded plugin %s\n", element->data->name);

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
	char *name = NULL;
	
	DbPlugElement *element = NULL;
	
	element = (DbPlugElement *) data;
	name		= (char *) func_data;

	//printf ("Cmp %s <=> %s?\n", name, element->data->name);

	return (strcmp (element->data->name, name));
}

char *
db_plug_get_by_name (char *name)
{
	DbPlugElement *element = NULL;
	
	element = (DbPlugElement *) db_list_find (pluglist, get_plug, (void *) name);

	if (element)
		return (element->data->data);
	else
		return ("ERROR");
}
