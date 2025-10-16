#if !defined(ERROR_H)
#define ERROR_H

void error_sys(const char *, const char *);
const char *error_what(void);
void error(const char *, const char *, ...);


#endif // ERROR_H
