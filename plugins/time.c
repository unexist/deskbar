#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "libdeskbar/log.h"
#include "libdeskbar/times.h"
#include "libdeskbar/plug.h"

static DbPlug *plug = NULL;

static char data[50];

void
time_update (void *options)
{
	strftime (data, sizeof (data), (char *) options, db_time_local ());
}

static DbPlug plugin =
{
	"Time",					/* Plugin name 							*/
	NULL,						/* Plugin create function		*/
	time_update,		/* Plugin update function		*/
	NULL,						/* Plugin destroy function	*/

	&data,					/* Plugin data							*/
	NULL,						/* Plugin format						*/
	
	2								/* Plugin update interval		*/
};

DbPlug *
db_plug_init (void)
{
	plug = &plugin;

	return (&plugin);
}
