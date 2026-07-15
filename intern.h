#ifndef INTERN_H
#define INTERN_H

// retorna um ponteiro estável para a string, nunca deve ser liberado
const char *intern(const char *s);

void intern_free(void);

#endif