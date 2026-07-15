#ifndef SYMTABLE_H
#define SYMTABLE_H

typedef struct SymbolEntry {
    char *name;
    char *type;
    struct SymbolEntry *next; // encadeamento dentro do escopo
} SymbolEntry;

typedef struct Scope {
    SymbolEntry *entries;
    struct Scope *parent; // escopo que contém este
} Scope;

typedef struct {
    Scope *current;
} SymTable;

SymTable *symtable_new(void);
void symtable_free(SymTable *st);

void symtable_enter_scope(SymTable *st);
void symtable_exit_scope(SymTable *st);

// retorna 0 se já existe no escopo atual
int symtable_add(SymTable *st, const char *name, const char *type);

// busca subindo pelos escopos (retorna NULL se não encontrar)
const char *symtable_lookup(SymTable *st, const char *name);

#endif