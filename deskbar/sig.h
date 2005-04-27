#ifndef DB_SIG_H
#define DB_SIG_H 1

extern jmp_buf env;

void db_sig_register (void);
void db_sig_destroy (void);

#endif /* DB_SIG_H */
