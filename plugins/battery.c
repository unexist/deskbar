#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libdeskbar/log.h"
#include "libdeskbar/times.h"
#include "libdeskbar/plug.h"

static DbPlug *plug = NULL;

static unsigned int bat_full = 0,
										bat_slot = 0;

static char data[40];

static FILE *fd1;

void
battery_create (void)
{
	FILE *fd2;
	
	char buf[256];
	
	/* Battery slot 0 */
  fd2				= fopen ("/proc/acpi/battery/BAT0/info", "r");
	bat_slot	= 0;

	/* Falling back to battery slot 1 */
	if (!fd2)
		{
			fd2				= fopen ("/proc/acpi/battery/BAT1/info", "r");
			bat_slot	= 1;
		}

	if (!fd2)
		{
			db_log_err ("Unable to open ACPI power iface? Disabling further checks!\n");

			return;
		}
	
  while (!feof (fd2))
		{
 			if (!fgets (buf, 255, fd2))
   			break;

	    if (!strncmp (buf, "last full capacity", 18)) 
				sscanf (buf, "last full capacity: %d", &bat_full);
		}
		
	db_log_debug ("Battery in Slot %d, Capacity %dmWh\n", bat_slot, bat_full);
	
	fclose (fd2);
}

void
battery_update (void *attrs)
{
	int capacity	= 0,
			percent		= 0;
	
	char buf[100],
			 state[20];

	/* Get battery info */
  if (!fd1)
		{
			snprintf (buf, sizeof (buf), "/proc/acpi/battery/BAT%d/state", bat_slot);
			
	    fd1 = fopen (buf, "r");

			memset (buf, 0, sizeof (buf));
		}
  else
    fseek (fd1, 0, SEEK_SET);
		
	if (!fd1)
		return;

  while (!feof (fd1))
		{
    	if (!fgets (buf, 100, fd1))
      	break;

	    if (!strncmp (buf, "charging state", 14)) 
				sscanf (buf, "charging state: %18s", &state);
			else
				if (!strncmp (buf, "remaining capacity", 18))
					sscanf (buf, "remaining capacity: %d", &capacity);
		}
		
	percent = ((double) capacity * 100.0) / bat_full;

	snprintf (data, sizeof (data), "%d%% (%s)", 
						percent, (!strncmp (state, "charging", 8)) ? "AC" : "BAT");
}

void
battery_destroy (void)
{
	if (fd1)
		fclose (fd1);
}

static DbPlug plugin =
{
	"Battery",					/* Plugin name 							*/
	battery_create,			/* Plugin create function		*/
	battery_update,			/* Plugin update function		*/
	battery_destroy,		/* Plugin destroy function	*/

	&data,							/* Plugin data							*/
	NULL,								/* Plugin format						*/

	3600								/* Plugin update interval		*/
};

DbPlug *
db_plug_init (void)
{
	plug = &plugin;

	return (&plugin);
}
