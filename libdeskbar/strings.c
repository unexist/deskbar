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

int
db_strings_ncase_cmp (int n,
	char *s1,
	char *s2)
{
	int c1, c2;
	
	while (n && *s1 && *s2)
		{
			n--;

			c1 = (int) is_lower (*s1);
			c2 = (int) is_lower (*s2);

			if (c1 != c2)
				return (c1 - c2);

			s1++;
			s2++;
		 }
	
	if (n)
		return ((int) *s1 - (int) *s2);
	else
		return (0);
}

void
db_strings_tolower (char *s)
{
	while (*s)
		{
			*s = is_lower (*s);
			s++;
		}
		
	return (s);
}

void
db_strings_toupper (char *s)
{
	while (*s)
		{
			*s = is_upper (*s);
			s++;
		}

	return (s);
}

/* djb2 hash algorithm */
unsigned long
db_strings_hash (char *s)
{
	unsigned long hash = 5381;
	int c;

	while (c = *s++)
		hash = ((hash << 5) + hash) + c;
		
	/* Some logic.. */
	hash += ~(hash << 9);
	hash ^=  ((hash >> 14) | (hash << 18));
	hash +=  (hash << 4);
	hash ^=  ((hash >> 10) | (hash << 22));
	
	return (hash);
}

