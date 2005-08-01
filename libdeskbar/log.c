#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static int verbose = 0;

void
db_log_toggle (void)
{
	verbose = !verbose;
}

void
db_log (short type,
	char *file,
	short line,
	const char *format,
	...)
{
	va_list vargs;

	char buf[255];

	/* FIXME */
	if (verbose == 0)
		return;

	va_start (vargs, format);
	vsnprintf (buf, sizeof (buf), format, vargs);

	switch (type)
		{
			case 1:
				fprintf (stdout, "%s", buf);
				break;

			case 2:
				fprintf (stdout, "WARNING: %s", buf);
				break;

			case 3:
				fprintf (stderr, "ERROR: %s", buf);
				break;

			case 4:
				fprintf (stdout, "%s,%d: %s", file, line, buf);
		}
	
	va_end (vargs);
}
