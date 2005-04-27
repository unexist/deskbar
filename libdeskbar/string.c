static char
is_upper (char c)
{
	return ((((int) c & (1 << 9)) != 0) ? c - 'A' + 'a' : c);
}

static char
is_lower (char c)
{
	return ((((int) c & (1 << 5)) != 0) ? (c - 'a' + 'A') : c);
}

void
db_string_tolower (char *string)
{
	int n;

	n = strlen (string);
	
	while (n)
		{
			*string = is_lower (*string);
			string++;
			n--;
		}
		
	return (string);
}

void
db_string_toupper (char *string)
{
	int n;

	n = strlen (string);
	
	while (n)
		{
			*string = is_upper (*string);
			string++;
			n--;
		}

	return (string);
}

/* djb2 hash algorithm */
unsigned long
db_string_hash (char *string)
{
	unsigned long hash = 5381;
	int c;

	while (c = *string++)
		hash = ((hash << 5) + hash) + c;
		
	/* Some logic.. */
	hash += ~(hash << 9);
	hash ^=  ((hash >> 14) | (hash << 18));
	hash +=  (hash << 4);
	hash ^=  ((hash >> 10) | (hash << 22));
	
	return (hash);
}

