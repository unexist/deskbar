#ifndef DB_LOG_H
#define DB_LOG_H 1

#define db_log_mesg(...) \
	db_log (1, __FILE__, __LINE__, __VA_ARGS__);

#define db_log_warn(...) \
  db_log (2, __FILE__, __LINE__, __VA_ARGS__);

#define db_log_err(...) \
	db_log (3, __FILE__, __LINE__, __VA_ARGS__);

#define db_log_debug(...) \
	db_log (4, __FILE__, __LINE__, __VA_ARGS__);

void db_log_toggle (void);
void db_log (short type, char *file, short line, const char *format, ...);

#endif /* DB_LOG_H */
