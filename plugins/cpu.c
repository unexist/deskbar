#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libdeskbar/log.h"
#include "libdeskbar/times.h"
#include "libdeskbar/plug.h"

static DbPlug *plug = NULL;

static double cpu_use		= 0.0,
							cpu_sum		= 0.0,
							cpu_delta	= 0.0,
							cpu_last	= 0.0;

static char data[50];					

static FILE *fd;

static void
get_stat (unsigned int *user,
	unsigned int *nice,
	unsigned int *system)
{
  char buf[256];
	
  if (!fd)
    fd = fopen ("/proc/stat", "r");
  else
    fseek (fd, 0, SEEK_SET);
		
  if (!fd)
		return;

  while (!feof (fd))
		{
    	if (!fgets (buf, 255, fd))
      	break;

	    if (!strncmp (buf, "cpu ", 4)) 
				sscanf (buf, "%*s %u %u %u", user, nice, system);
    }
}

void
cpu_create (void)
{
	cpu_last = db_time_current ();
}

void
cpu_update (void *attrs)
{
	unsigned int user		= 0, 
							 nice		= 0,
							 system	= 0; 
	
	cpu_delta = db_time_update_time () - cpu_last;
	cpu_last	= db_time_update_time ();
	
	get_stat (&user, &nice, &system);

	cpu_use	= (user + nice + system - cpu_sum) / cpu_delta / 100.0;
	cpu_sum = (user + nice + system);

	snprintf (data, sizeof (data), "%d%%", (int) (cpu_use * 101.0));
}

void
cpu_destroy (void)
{
	if (fd)
		fclose (fd); 
}

static DbPlug plugin =
{
	"CPU",					/* Plugin name 							*/
	cpu_create,			/* Plugin create function		*/
	cpu_update,			/* Plugin update function		*/
	cpu_destroy,		/* Plugin destroy function	*/

	&data,					/* Plugin data							*/
	NULL,						/* Plugin format						*/

	6								/* Plugin update interval		*/
};

DbPlug *
db_plug_init (void)
{
	plug = &plugin;
	
	return (&plugin);
}
