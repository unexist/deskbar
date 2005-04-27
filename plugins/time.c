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
time_create (void)
{
	if (!plug->format)
		plug->format = "%H:%M";
}

void
time_update (void)
{
	strftime (data, sizeof (data), plug->format, db_time_local ());
}

static DbPlug plugin =
{
	"Time",					/* Plugin name 							*/
	time_create,		/* Plugin create function		*/
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
