#ifndef DB_LPNG_H
#define DB_LPNG_H 1

void db_lpng_version (void);

int db_lpng_check_sig (FILE *file, unsigned long *w, unsigned long *h);

unsigned char *db_lpng_load (double display_exponent, int *channels, 
														 unsigned long *bytes);

void db_lpng_cleanup (int clean);

#endif /* DB_LPNG_H */
