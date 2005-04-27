#ifndef DB_PLUG_H
#define DB_PLUG_H 1

typedef struct dbplug_t
{
	char *name;
	
	void (*create) (void);
	void (*update) (void);
	void (*destroy) (void);

	char *data;
	char *format;

	int interval;	
} DbPlug;

#endif /* DB_PLUG_H */
