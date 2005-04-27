#ifndef DB_TIME_H
#define DB_TIME_H 1

double db_time_current (void);
double db_time_delta (void);
double db_time_update_time (void);
double db_time_update_diff (void);

void db_time_update (void);

struct tm *db_time_local (void);

#endif /* DB_LIB_TIME_H */
