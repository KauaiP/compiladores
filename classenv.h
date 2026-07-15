#ifndef CLASSENV_H
#define CLASSENV_H

#include "ast.h"

typedef struct MethodEntry {
    char *name;
    char **param_types;   // tipos dos formals
    int param_count;
    char *return_type;
    struct MethodEntry *next;
} MethodEntry;

typedef struct AttrEntry {
    char *name;
    char *type;
    struct AttrEntry *next;
} AttrEntry;

typedef struct ClassEntry {
    char *name;
    char *parent;
    MethodEntry *methods;
    AttrEntry *attrs;
    struct ClassEntry *next;
} ClassEntry;

typedef struct {
    ClassEntry *classes;
} ClassEnv;

ClassEnv *classenv_new(void);
void classenv_free(ClassEnv *env);

// registra uma classe (retorna 0 se já existe)
int classenv_add_class(ClassEnv *env, const char *name, const char *parent);

// adiciona método ou atributo a uma classe
int classenv_add_method(ClassEnv *env, const char *class_name, const char *method_name, char **param_types, int param_count, const char *return_type);
int classenv_add_attr(ClassEnv *env, const char *class_name, const char *attr_name, const char *type);

// lookups
ClassEntry *classenv_lookup_class(ClassEnv *env, const char *name);
MethodEntry *classenv_lookup_method(ClassEnv *env, const char *class_name, const char *method_name);
AttrEntry *classenv_lookup_attr(ClassEnv *env, const char *class_name, const char *attr_name);

//verifica se a existe herança cíclica
int classenv_check_cycles(ClassEnv *env);

// retorna 1 se child é subtipo de parent
int classenv_is_subtype(ClassEnv *env, const char *child, const char *parent);

// least upper bound de dois tipos
const char *classenv_lub(ClassEnv *env, const char *a, const char *b);

#endif