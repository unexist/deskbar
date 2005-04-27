#ifndef DB_STRING_H
#define DB_STRING_H 1

int db_strings_ncase_cmp (int n, char *s1, char *s2);

void db_strings_tolower (char *s;
void db_strings_toupper (char *s);

unsigned long db_strings_hash (char *s);

#endif /* DB_STRING_H */
