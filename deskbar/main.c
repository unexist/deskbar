#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "display.h"
#include "plug.h"
#include "sig.h"
#include "xml.h"

#include "libdeskbar/log.h"
#include "libdeskbar/mem.h"

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

static void
usage (char *prog)
{
  db_log_mesg ("Usage: %s [OPTIONS]\n\n" \
							 "Options:\n" \
							 "  -c, --config    \t Load this rc file (default: ~/.deskbarrc)\n" \
							 "  -f, --foreground\t Stay in foreground!\n" \
							 "  -p, --profile   \t Show memory profile\n",
							 prog);

	db_log_mesg ("  -V, --verbose   \t Be verbosiv\n" \
							 "  -v, --version   \t Display version information and exit\n" \
							 "  -h, --help      \t Display this help and exit\n\n" \
							 "Please report bugs to <%s>\n",
							 prog, PACKAGE_BUGREPORT);
}

static void
version (void)
{
	db_log_mesg ("%s v%s - Copyright (c) 2004-2005 Christoph Kappel\n" \
							 "Released under the GNU Public License\n",
							 PACKAGE_NAME, PACKAGE_VERSION);
}

int
main (int argc,
	char *argv[])
{
	int c, pid;
	int fg = 0;

	char *file = NULL;

  static struct option long_options[] =
    {
			{"config",			required_argument,	0, 'c'},
			{"foreground",	no_argument,				0, 'f'},
			{"profile",     no_argument,        0, 'p'},
			{"verbose",			no_argument,				0, 'v'},
      {"version",			no_argument,				0, 'V'},
      {"help", 				no_argument,				0, 'h'},
      {0, 0, 0, 0}
    };

	/* Fetching getops */
  while ((c = getopt_long (argc, argv, "c:fpVvh", long_options, NULL)) != -1)
    {
      switch (c)
        {
					case 'c':
						if (optarg)
							file = optarg;
						break;

					case 'f':
						fg = 1;
						break;

					case 'p':
						db_mem_toggle_profile ();
						break;
						
					case 'v':
						db_log_toggle ();
						break;

					case 'V':
            version ();
						return (0);
						break;

          case 'h':
            usage (argv[0]);
        		return (0);
      			break;

          case '?':
						db_log_mesg ("Try `%s --help' for more information\n",
            						argv[0]);

						return (-1);
        }
    }

	version ();

	db_xml_version ();
	
	db_sig_register ();

	db_display_init ();
	db_plug_init ();
	
	db_xml_parse_file (file);

	db_display_draw ();

	if (fg != 1)
		{
			pid = fork ();

			switch (pid)
				{
					case -1:
						db_log_mesg ("Unable to fork to background!\n");
						db_log_debug ("fork (): %s\n", strerror (errno));
						break;

					case 0:
						break;

					default:
						db_log_mesg ("Forked to background (%d)\n", pid);
						exit (0);
				}
		}

	db_display_event_loop ();

	db_sig_destroy ();
	
	return (0);
}
