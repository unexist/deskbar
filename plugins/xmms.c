#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <xmms/xmmsctrl.h>

#include "libdeskbar/log.h"
#include "libdeskbar/times.h"
#include "libdeskbar/plug.h"

static DbPlug *plug = NULL;

static int session;

static char data[100];

static int
get_session (void)
{
	for (session = 0; session < 16; session++)
		if (xmms_remote_is_running (session))
			{
				db_log_debug ("Session id %d, Protocol version %d\n", 
													session, xmms_remote_get_version (session));

				return (1);
			}
	
	session = -1;

	return (0);
} 

void
xmms_create (void)
{
	if (!get_session ())
		db_log_debug ("XMMS is not running\n");
}

void
xmms_update (void)
{
	int num, time, mins, secs;
	
	char *title = NULL;
			 
	if (!xmms_remote_is_running (session))
		if (!get_session ())
			{
				snprintf (data, sizeof (data), "XMMS is not running");

				return;
			}
	
	num		= xmms_remote_get_playlist_pos (session) + 1;
	time	= (xmms_remote_get_output_time (session) / 1000);
	
	mins	= (int) (time / 60);
	secs	= (int) (time % 60);

	title = (xmms_remote_get_playlist_title (session, (num - 1)));

	snprintf (data, sizeof (data), "%d. %s [%d%d:%d%d]", 
						num, title,
						((mins <= 9) ? 0 : (mins / 10)), (mins % 10),
						((secs <= 9) ? 0 : (secs / 10)), (secs % 10));
		
	free (title);
}

static DbPlug plugin =
{
	"XMMS",					/* Plugin name 							*/
	xmms_create,		/* Plugin create function		*/
	xmms_update,		/* Plugin update function		*/
	NULL,						/* Plugin destroy function	*/

	&data,					/* Plugin data							*/
	NULL,						/* Plugin format						*/
	
	60							/* Plugin update interval		*/
};

DbPlug *
db_plug_init (void)
{
	plug = &plugin;

	return (&plugin);
}
