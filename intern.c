#include <stdlib.h>
#include <string.h>
#include "intern.h"

#define MAX_STRINGS 1024

static char *table[MAX_STRINGS];
static int count = 0;

const char *intern(const char *s) {
    for (int i = 0; i < count; i++)
        if (strcmp(table[i], s) == 0)
            return table[i];
    if (count >= MAX_STRINGS) {
        // nunca deve acontecer num compilador COOL
        return s;
    }
    table[count] = strdup(s);
    return table[count++];
}

void intern_free(void) {
    for (int i = 0; i < count; i++)
        free(table[i]);
    count = 0;
}