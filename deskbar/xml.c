#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "desk.h"
#include "plug.h"

#include "libdeskbar/htable.h"
#include "libdeskbar/log.h"

static char **
parse_line (char *line)
{
	int q = 0;
	
	int toks_len	= 5;
	int buf_len		= 2;

	char *buf		= NULL;
	char **toks	= NULL;

	int b = 0;
	int t = 0;

	do
		{
			switch (*line)
				{
					/* Skip comments */
					case '#':
						if (!q)
							return (NULL);
							
					/* Skip special chars */
					case '\t':
						break;
					
					/* Skip other chars if not quoted*/
					case '<':
					case '>':
					case '/':
					case '?':
						if (!q)
							break;

					/* Quotes */
					case '"':
						q = !q;
						break;
						
					case ' ':
					case '=':
					case '\n':
						if (!q)
							{
								if (buf)
									{
										toks_len += 5;
										toks			= (char **) realloc (toks, toks_len);

										if (toks)
											{
												toks[t++] = buf;
												toks[t]		= NULL;
												
												buf = NULL;
												b		= 0;
											}
									}

								break;
							}
						
					default:
						buf_len += 2;
						buf			 = (char *) realloc (buf, buf_len);

						if (buf)
							{
								buf[b++]	= *line;
								buf[b]		= '\0';
							}
				}
		}	
	while (*line++ != '\n');

	return (toks);
}

static void
handle_toks (char **toks)
{
	int i;

	DbHtable *table = NULL;
	
	if (toks)
		{
			table = db_htable_new ();

			for (i = 1; toks[i] != NULL; i++)
				{
					db_htable_push (table, (void *) toks[i], (void *) toks[i + 1]);
					i++;
				}

			if (!strcmp (toks[0], "plugin"))
				{
					/* Plugins */
					db_plug_load (
						db_htable_pop (table, "name"),
						db_htable_pop (table, "format"),
						atoi (db_htable_pop (table, "interval")));
				}
			else if (!strcmp (toks[0], "text"))
				{
					/* Texts */
					db_desk_obj_new (DB_OBJ_TEXT, table);

				}
			else if (!strcmp (toks[0], "meter"))
				{
					/* Meters */
					db_desk_obj_new (DB_OBJ_METER, table);
				}

			db_htable_destroy (table);
		}
}

static void
cleanup (char **toks)
{
	int i;

	if (toks)
		{
			for (i = 0; toks[i] != NULL; i++)
				free (toks[i]);
	
		free (toks);

		toks = NULL;
	}
}
					
int
db_xml_parse_file (char *rc_file)
{
	FILE *fd = NULL;

	char file[120];
	char buf[255];

	char **toks	= NULL;

	if (rc_file)
		snprintf (file, sizeof (file), "%s", rc_file);
	else
		snprintf (file, sizeof (file), "%s/.deskbarrc", getenv ("HOME"));

	db_log_mesg ("Reading %s\n", file);

	fd = fopen (file, "r");

	if (!fd)
		{
			switch (errno)
				{
					case EISDIR:
						db_log_err ("File `%s' is a directory?\n", file);
						break;

					case ENOENT:
						db_log_err ("Unable to find configuration!\n");
						break;

					default:
						db_log_debug ("Unhandled event? Damn..\n");
				}

			return (-1);
		}

	while (fgets (buf, sizeof (buf), fd))
		{
			toks = parse_line (buf);
			
			handle_toks (toks);

			cleanup (toks);
		}

	fclose (fd);

	return (0);
}
