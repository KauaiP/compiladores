#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symtable.h"
#include "intern.h"

SymTable *symtable_new(){
    SymTable *new = malloc(sizeof(SymTable));
    new->current = NULL;
    return new;
}

void symtable_exit_scope(SymTable *st){
    if (st->current == NULL)
    {
        return;
    }

    Scope* scope = st->current;

    SymbolEntry *entry = scope->entries;
    while (entry != NULL)
    {
        SymbolEntry *next = entry->next;
        free(entry->name);
        free(entry);
        entry = next;
    }
    
    st->current = scope->parent;
    free(scope);
}

void symtable_free(SymTable *st){
    while (st->current != NULL)
    {
        symtable_exit_scope(st);
    }
    free(st);
    
}

void symtable_enter_scope(SymTable *st){
    Scope *scope = malloc(sizeof(Scope));
    scope->entries = NULL;
    scope->parent = st->current;
    st->current = scope;
}

int exist(const char *name, const char *type, Scope *scope){
    SymbolEntry *entry = scope->entries;
    while (entry != NULL)
    {
        if (strcmp(entry->name, name) == 0)
        {
            return 1;
        }
        entry = entry->next;
    }

    return 0;
}

int symtable_add(SymTable *st, const char *name, const char *type){
    SymbolEntry *e = st->current->entries;
    while (e != NULL) {
        if (strcmp(e->name, name) == 0) return 0;
        e = e->next;
    }
    SymbolEntry *entry = malloc(sizeof(SymbolEntry));
    entry->name = strdup(name);
    entry->type = (char *)intern(type);
    entry->next = st->current->entries;
    st->current->entries = entry;
    return 1;
}

const char* symtable_lookup(SymTable *st, const char *name){
    Scope *scope = st->current;
    while (scope != NULL)
    {
        SymbolEntry *entry = scope->entries;
        while (entry != NULL)
        {
            if (strcmp(entry->name, name) == 0)
            {
                return entry->type;
            }
            entry = entry->next;
        }
        scope = scope->parent;
        
    }
    return NULL;
}
