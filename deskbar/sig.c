#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

#include "display.h"
#include "plug.h"
#include "sig.h"

#include "libdeskbar/log.h"
#include "libdeskbar/mem.h"

typedef struct
{
  int num;
  char *name;
} Signals;

static Signals signals[] =
{
  {SIGSEGV,   "SEGV"   },
  {SIGHUP,    "HUP"    },
  {SIGINT,    "INT"    },
  {SIGTERM,   "TERM"   },
  {SIGQUIT,   "QUIT"   },
  {-1,      "Unknown"  }
};

jmp_buf env;

static void
sig_handler (int sig)
{
  switch (sig)
    {
      case SIGSEGV:
				longjmp (env, 1);	

				db_log_debug ("Something went wrong! Segmentation fault!\n");
				db_sig_destroy ();

				abort ();
        break;

      default:
				db_log_mesg ("Caught signal SIG%s\n", signals[sig].name);
				db_sig_destroy ();
				
        exit (0);
    }
}

void
db_sig_register (void)
{
	int i;
		
  for (i = 0; signals[i].num != -1; ++i)
    if (signal (signals[i].num, sig_handler) == SIG_ERR)
      db_log_mesg ("Couldn't set signal %s (%d) for catching",
									 signals[i].name, signals[i].num);
}

void
db_sig_destroy (void)
{
	db_mem_profile ();

	db_plug_unload_all ();
	db_display_destroy ();
	
	db_mem_table_destroy ();
}
