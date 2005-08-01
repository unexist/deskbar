#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "display.h"
#include "plug.h"

#include <libxml/xmlversion.h>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>

#include "libdeskbar/htable.h"
#include "libdeskbar/log.h"

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

static void
error_handler (void *arg,
	const char *msg,
	xmlParserSeverities severity,
	xmlTextReaderLocatorPtr locator)
{
	int line;
	unsigned int n, col;
	
	const xmlChar *cur	= NULL;
	const xmlChar *base = NULL;

	xmlChar *ctnt = NULL;
	xmlChar content[81];

	xmlParserCtxtPtr ctx;

	line = xmlTextReaderLocatorLineNumber (locator);

	/* Heavily based on the internal libxml error handler */
	ctx = (xmlParserCtxtPtr) locator;
	cur = ctx->input->cur;
	
	while ((cur > base) && ((*(cur) == '\n') || (*(cur) == '\r'))) 
	  cur--;
	n = 0;
	
	while ((n++ < (sizeof (content) - 1)) && (cur > base) &&
		(*(cur) != '\n') && (*(cur) != '\r'))
		cur--;
		
	if ((*(cur) == '\n') || (*(cur) == '\r')) 
		cur++;
	
	col = ctx->input->cur - cur;
	
	n = 0;
	ctnt = content;
	
	while ((*cur != 0) && (*(cur) != '\n') &&
		(*(cur) != '\r') && (n < sizeof (content) - 1)) 
	{
		*ctnt++ = *cur++;
  	n++;
	}
	
	*ctnt = 0;
	
	db_log_err ("%s\n", content);
		
	n = 0;
	ctnt = content;
	
	while ((n < col) && (n++ < sizeof (content) - 2) && (*ctnt != 0))
		{
			if (*(ctnt) != '\t')
				*(ctnt) = ' ';
				ctnt++;
		}
		
	*ctnt++ = '^';
	*ctnt		= 0;
	
	db_log_err ("%s\n", content);
		
	switch (severity)
		{
			case 3:
			case 4:
				db_log_err ("Line %d, Char %d: %s", line, col, msg);
				break;

			default:
				db_log_debug ("Validation not implemented yet\n");
		}
}

static void
parse_node (xmlTextReaderPtr reader)
{
	xmlChar *name		= NULL;
	xmlChar *attrib	= NULL;
	xmlChar *value	= NULL;

	DbHtable *table = NULL;
	
	name = xmlTextReaderLocalName (reader);

	if (xmlTextReaderHasAttributes (reader))
		{
			xmlTextReaderMoveToFirstAttribute (reader);

			table = db_htable_new ();

			do
				{
					attrib 	= xmlTextReaderLocalName (reader);
					value		= xmlTextReaderValue (reader);

					db_htable_push (table, (void *) attrib, (void *) value);

					xmlFree (attrib);
					xmlFree (value);
				}
			while (xmlTextReaderMoveToNextAttribute (reader));
		
		/* Checking configuration version */
		if (!strcmp (name, "display"))
			{
				if (strcmp (db_htable_pop (table, "version"), PACKAGE_VERSION))
					{
						db_log_warn ("Configuration version mismatch: %s vs. %s!\n", 
							db_htable_pop (table, "version"), PACKAGE_VERSION)
					}
			}
		else if (!strcmp (name, "plugin"))
			{
				db_plug_load (table);
			}
		else if (!strcmp (name, "text"))
			{
				db_display_obj_new (DB_OBJ_TEXT, table);
			}
		else if (!strcmp (name, "meter"))
			{
				db_display_obj_new (DB_OBJ_METER, table);
			}

		db_htable_destroy (table);
	}
	
	xmlFree (name);
}

void
db_xml_version (void)
{
	LIBXML_TEST_VERSION;

	db_log_mesg ("Using libxml v%s\n", LIBXML_DOTTED_VERSION);
}

int
db_xml_parse_file (char *rc_file)
{
	int ret;

	char file[100];
	
	xmlTextReaderPtr reader;
	
	if (rc_file)
		snprintf (file, sizeof (file), "%s", rc_file);
	else
		snprintf (file, sizeof (file), "%s/.deskbarrc", getenv ("HOME"));

	db_log_mesg ("Reading %s\n", file);
	
	reader = xmlReaderForFile (&file, NULL, 0);

	/* Set error handler */
	xmlTextReaderSetErrorHandler (reader, error_handler, NULL);
	
	if (reader) 
		{
			ret = xmlTextReaderRead (reader);
			
			while (ret == 1)
				{
					parse_node (reader);
					ret = xmlTextReaderRead (reader);
				}
				
			xmlFreeTextReader (reader);
	
			if (ret != 0)
				db_log_err ("Failed to parse `%s'!\n", file);
		}
	else
		{
			db_log_err ("Unable to open `%s'!\n", file);
		}
		
	xmlCleanupParser ();
	xmlMemoryDump ();

	return (0);
}
