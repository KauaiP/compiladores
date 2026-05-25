#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symtable.h"

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
        free(entry->type);
        free(entry);
        entry = next;
    }
    
    st->current = scope->parent;
    free(scope);
}

void symtable_free(SymTable *st){
    while (st->current != NULL)
    {
        syntable_exit_scope(st);
    }
    free(st);
    
}

void symtable_enter_scope(SymTable *st){
    Scope *scope = malloc(sizeof(Scope));
    scope->entries = NULL;
    scope->parent = st->current;
    st->current = scope;
}

int symtable_add(SymTable *st, const char *name, const char *type){
    if (exist(name, type, st->current))
    {
        return 0;
    }
    
    SymbolEntry *newEntry = malloc(sizeof(SymbolEntry));
    newEntry->name = name;
    newEntry->type = type;
    newEntry->next = st->current->entries;
    st->current->entries = newEntry;
    return 1;
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
