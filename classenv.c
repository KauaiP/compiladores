#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "classenv.h"

ClassEnv *classenv_new(){
    ClassEnv *env = malloc(sizeof(ClassEnv));
    env->classes = NULL;
    
    classenv_add_class(env, "Object", NULL);
    classenv_add_class(env, "IO", "Object");
    classenv_add_class(env, "Int", "Object");
    classenv_add_class(env, "String", "Object");
    classenv_add_class(env, "Bool", "Object");

    return env;
}

void classenv_free(ClassEnv *env){
    if (!env)
    {
        return;
    }
    
    ClassEntry *classEntry = env->classes;
    while (classEntry != NULL)
    {
        ClassEntry *next = classEntry->next;
        free(classEntry->name);
        free(classEntry->parent);

        MethodEntry *methodEntry = classEntry->methods;
        AttrEntry *attrEntry = classEntry->attrs;
    
        while (methodEntry != NULL)
        {
            MethodEntry *next = methodEntry->next;
            free(methodEntry->name);
            free(methodEntry->return_type);
            for (int i = 0; i < methodEntry->param_count; i++)
                free(methodEntry->param_types[i]);
            free(methodEntry->param_types);
            free(methodEntry);
            methodEntry = next;
        }

        while (attrEntry != NULL)
        {
            AttrEntry *next = attrEntry->next;
            free(attrEntry->name);
            free(attrEntry->type);
            free(attrEntry);
            attrEntry = next;
        }

        free(classEntry);
        classEntry = next;
        
    }
    free(env);
}

int classenv_add_class(ClassEnv *env, const char *name, const char *parent){
    if (classenv_lookup_class(env, name) != NULL)
    {
        return 0;
    }
    
    ClassEntry *new_class = malloc(sizeof(ClassEntry));
    new_class->attrs = NULL;
    new_class->methods = NULL;
    new_class->name = strdup(name);
    new_class->parent = parent ? strdup(parent) : NULL;
    new_class->next = env->classes;
    env->classes = new_class;
    return 1;
}

int classenv_add_method(ClassEnv *env, const char *class_name, const char *method_name, char **param_types, int param_count, const char *return_type){
    ClassEntry *class = classenv_lookup_class(env, class_name);
    if (class == NULL)
    {
        return 0;
    }

    MethodEntry *aux = class->methods;
    while (aux != NULL) {
        if (strcmp(aux->name, method_name) == 0){
            return 0;
        }
        aux = aux->next;
    }
    
    MethodEntry *method = malloc(sizeof(MethodEntry));
    method->name = strdup(method_name);
    method->param_types = malloc(sizeof(char*) * param_count);
    method->return_type = strdup(return_type);
    method->param_count = param_count;

    for (int i = 0; i < param_count; i++)
    {
        method->param_types[i] = param_types[i];
    }

    method->next = class->methods;
    class->methods = method;
    return 1;

}

int classenv_add_attr(ClassEnv *env, const char *class_name, const char *attr_name, const char *type){
    ClassEntry *class = classenv_lookup_class(env, class_name);
    if (class == NULL)
    {
        return 0;
    }
    
    AttrEntry *aux = class->attrs;
    while (aux != NULL)
    {
        if (strcmp(aux->name, attr_name) == 0)
        {
            return 0;
        }
        aux = aux->next;
    }

    AttrEntry *attr = malloc(sizeof(AttrEntry));
    attr->name = strdup(attr_name);
    attr->type = strdup(type);
    attr->next = class->attrs;
    class->attrs = attr;
    return 1;
    
}


ClassEntry *classenv_lookup_class(ClassEnv *env, const char *name){
    ClassEntry *classEntry = env->classes;
    while (classEntry != NULL)
    {
        if (strcmp(classEntry->name, name) == 0)
        {
            return classEntry;
        }
        classEntry = classEntry->next;
    }
    return NULL;
}

MethodEntry *classenv_lookup_method(ClassEnv *env, const char *class_name, const char *method_name) {
    const char *current = class_name;
    while (current != NULL) {
        ClassEntry *cls = classenv_lookup_class(env, current);
        if (cls == NULL)
        {
            break;
        }

        MethodEntry *m = cls->methods;
        while (m != NULL) {
            if (strcmp(m->name, method_name) == 0)
            {
                return m;
            }
            m = m->next;
        }

        current = cls->parent;
    }
    return NULL;
}

AttrEntry *classenv_lookup_attr(ClassEnv *env, const char *class_name, const char *attr_name) {
    const char *current = class_name;
    while (current != NULL) {
        ClassEntry *cls = classenv_lookup_class(env, current);
        if (cls == NULL)
        {
            break;
        }

        AttrEntry *a = cls->attrs;
        while (a != NULL) {
            if (strcmp(a->name, attr_name) == 0)
            {
                return a;
            }
            a = a->next;
        }

        current = cls->parent;
    }
    return NULL;
}

int classenv_check_cycles(ClassEnv *env) {
    ClassEntry *cls = env->classes;
    while (cls != NULL) {
        // sobe a cadeia a partir dessa classe
        const char *current = cls->name;
        int steps = 0;
        int total = 0;

        // conta quantas classes existem
        ClassEntry *tmp = env->classes;
        while (tmp != NULL) { total++; tmp = tmp->next; }

        while (current != NULL) {
            ClassEntry *entry = classenv_lookup_class(env, current);
            if (entry == NULL) break;           // chegou em tipo desconhecido
            if (entry->parent == NULL) break;   // chegou em Object — sem ciclo
            current = entry->parent;
            steps++;
            if (steps > total) return 1;        // andou mais do que o total de classes — ciclo
        }
        cls = cls->next;
    }
    return 0;
}

int classenv_is_subtype(ClassEnv *env, const char *child, const char *parent) {
    const char *current = child;
    while (current != NULL) {
        if (strcmp(current, parent) == 0)
        {
            return 1;
        }
        ClassEntry *cls = classenv_lookup_class(env, current);
        if (cls == NULL){
            break;
        } 
        current = cls->parent;
    }
    return 0;
}

const char *classenv_lub(ClassEnv *env, const char *a, const char *b) {
    const char *current = a;
    while (current != NULL) {
        if (classenv_is_subtype(env, b, current))
        {
            return current;
        }
        ClassEntry *cls = classenv_lookup_class(env, current);
        if (cls == NULL)
        {
            break;
        }
        current = cls->parent;
    }
    return "Object";
}
