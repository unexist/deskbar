#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#include "display.h"
#include "plug.h"

#include "libdeskbar/list.h"
#include "libdeskbar/log.h"
#include "libdeskbar/strings.h"
#include "libdeskbar/htable.h"
#include "libdeskbar/times.h"

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

typedef struct xserver_t
{
	Display *display;
	Window root;

	Colormap colormap;
} XServer;

typedef struct object_t
{
	int x;
	int y;
	int w;
	int h;
	
	GC gc;

	DbObjType type;

	char *value;
	char *data;
} Object;

static XServer *xs			= NULL;
static DbList *objlist	= NULL;

static long
parse_col (const char *color_name)
{
	XColor color;

	color.pixel = 0;

	if (!XParseColor (xs->display, xs->colormap, color_name, &color))
		{
			db_log_err ("Unable to parse color `%s'\n", color_name);
		}
	else if (!XAllocColor (xs->display, xs->colormap, &color))
		db_log_err ("Unable to allocate color `%s'\n", color_name);
	
	return ((long) color.pixel);
}

static XFontStruct *
parse_font (const char *font_name,
	int font_size)
{
	char new[50];
	
	XFontStruct *xfs = NULL;

	snprintf (new, sizeof (new), 
		"-*-%s-*-*-*-*-%d-*-*-*-*-*-*-*", font_name, font_size);

	xfs = XLoadQueryFont (xs->display, new);

	if (!xfs)
		{
			db_log_err ("Unable to parse font `%s', "\
				"falling back to fixed.\n", 
				font_name);

			xfs = XLoadQueryFont (xs->display, 
				"-*-fixed-medium-*-*-*-12-*-*-*-*-*-*-*");
		}
	
	return (xfs);
}

void
db_display_init (void)
{
	int release, screen;
			
	xs = (XServer *) malloc (sizeof (XServer));

	xs->display = XOpenDisplay (NULL);
	
	screen	= DefaultScreen (xs->display);
	release	= VendorRelease (xs->display);
	
	xs->root			= RootWindow (xs->display, screen);
	xs->colormap	= DefaultColormap (xs->display, screen);

	db_log_mesg ("X Version %d, Revision %d, Release %d.%d.%d\n" \
							 "Display Geometry %dx%d @ %dbit\n",
							 ProtocolVersion (xs->display),
							 ProtocolRevision (xs->display),
							 (release / 10000000), 
							 (release % 10000000 / 100000),
							 (release % 10000000 % 100000 / 1000),
							 DisplayWidth (xs->display, screen),
							 DisplayHeight (xs->display, screen),
							 DefaultDepth (xs->display, screen));

	/* Listen on property changes of the root window */
	XSelectInput (xs->display, xs->root, ExposureMask);

	objlist = db_list_new ();	
}

void
db_display_obj_new (DbObjType type,
	DbHtable *table)
{
	char *data	= NULL;
	char *value = NULL;
	
	XGCValues gc_vals;
	
	XFontStruct *xfs = NULL;
	
	Object *obj = NULL;

	obj = (Object *) calloc (1, sizeof (Object));
	
	obj->x		= atoi (db_htable_pop (table, "x"));
	obj->y		= atoi (db_htable_pop (table, "y"));
	obj->type	= type;

	data	= db_htable_pop (table, "data");
	value	= db_htable_pop (table, "value");

	switch (type)
		{
			case DB_OBJ_TEXT:
				xfs	= parse_font (
					db_htable_pop (table, "font"),
					atoi (db_htable_pop (table, "size")));

				obj->h = xfs->ascent + xfs->descent;

				if (value)
					obj->value	= strdup (value);
				else if (data)
					obj->data = db_plug_get_by_name (data);
				else
					{
						db_log_err ("Malformed configuration data!\n");
						
						return;
					}

				gc_vals.graphics_exposures	= False;
				gc_vals.foreground					= parse_col (db_htable_pop (table, "color"));
				gc_vals.font								= xfs->fid;

				obj->gc = XCreateGC (xs->display, xs->root, 
					GCFont|GCForeground|GCGraphicsExposures, &gc_vals);

				XFreeFont (xs->display, xfs);
				break;

			case DB_OBJ_METER:
				obj->w = atoi (db_htable_pop (table, "width"));
				obj->h = atoi (db_htable_pop (table, "height"));

				if (data)
					obj->data = db_plug_get_by_name (data);
				else
					{
						db_log_err ("Malformed configuration data!\n");
						
						return;
					}

				gc_vals.graphics_exposures	= False;
				gc_vals.foreground					= parse_col (db_htable_pop (table, "color"));
				
				obj->gc = XCreateGC (xs->display, xs->root, 
					GCForeground|GCGraphicsExposures, &gc_vals);
				
				break;
		}

	db_list_insert (objlist, (void *) obj);
}

static void
draw_objects (void *data,
	void *user_data)
{
	Object *obj = NULL;

	obj = (Object *) data;

	switch (obj->type)
		{
			case DB_OBJ_TEXT:
				XDrawString (xs->display, xs->root, obj->gc,
					obj->x, (obj->y + obj->h), 
					(obj->data) ? obj->data : obj->value, 
					strlen ((obj->data) ? obj->data : obj->value));
				break;

			case DB_OBJ_METER:
				XDrawRectangle (xs->display, xs->root, obj->gc,
					obj->x, obj->y, obj->w, obj->h);
				XFillRectangle (xs->display, xs->root, obj->gc,
					(obj->x + 2), (obj->y + 2), (((obj->w - 3) * atoi (obj->data)) / 100),
					(obj->h - 3));
				break;
		}
}

void 
db_display_draw (void)
{
	XClearWindow (xs->display, xs->root);

	db_plug_update ();
	db_list_foreach (objlist, draw_objects, NULL);

	XFlush (xs->display);
}

static void
destroy_object (void *data,
	void *user_data)
{
	Object *obj = NULL;

	obj = (Object *) data;

	XFreeGC (xs->display, obj->gc);

	if (obj->value)
		free (obj->value);

	free (obj);
}

void
db_display_destroy (void)
{
	XClearWindow (xs->display, xs->root);
	XFlush (xs->display);

	db_list_destroy (objlist, destroy_object);

	XFreeColormap (xs->display, xs->colormap);

	free (xs);
}

void
db_display_event_loop (void)
{
	int s;
	
  XEvent event;
	
	struct timeval tv;
	
  while (1)
		{
	    /* Wait for X event or timeout */
  	  if (!XPending (xs->display))
				{
	      	fd_set fdset;
					
					tv.tv_sec		= db_plug_get_interval ();
					tv.tv_usec	= 0;

					FD_ZERO (&fdset);
					FD_SET (ConnectionNumber (xs->display), &fdset);

					s = select (ConnectionNumber (xs->display) + 1, &fdset, 0, 0, &tv);

					switch (s)
						{
							case -1:
  	      			if (errno != EINTR)
    	      			db_log_debug ("Unable to select %s", strerror (errno));
								break;

							/* Timeout */
							case 0:
								db_display_draw ();
								break;
						}
	  	  }

			/* Handle X events */
			while (XPending (xs->display)) 
				{
	    		XNextEvent (xs->display, &event);
			
		  	  switch (event.type)
						{
  		    		case Expose:
								if (event.xexpose.window == xs->root)
      		    		db_display_draw ();
			      	  break;
						}
				}
		}
}
