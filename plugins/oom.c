#include <string.h>

#include "libdeskbar/log.h"
#include "libdeskbar/mem.h"
#include "libdeskbar/plug.h"

#define MAX 5

static DbPlug *plug = NULL;
static int *oom			= NULL;

static int counter = 1;

void
oom_create (void)
{
	db_log_debug ("ATTENTION: Mem-table and exception-handling test!\n");
}

void
oom_update (void *attrs)
{
	int i;

	if (counter == MAX)
		{
			char *foo = NULL;
			strcpy (foo, "Error");
		}
	else
		{
			oom = db_mem_realloc (oom, (sizeof (int) * counter)); 

			for (i = 0; i < counter; i++)
				oom[i] = i;
		}

	counter++;
}

static DbPlug plugin =
{
	"OOM",					/* Plugin name 							*/
	oom_create,			/* Plugin create function		*/
	oom_update,			/* Plugin update function		*/
	NULL,						/* Plugin destroy function	*/

	NULL,						/* Plugin data							*/
	NULL,						/* Plugin format						*/
	
	2								/* Plugin update interval		*/
};

DbPlug *
db_plug_init (void)
{
	plug = &plugin;

	return (&plugin);
}
