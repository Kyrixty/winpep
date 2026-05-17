#include <stdio.h>
#include "coff.h"
#include "arena.h"

#define QUERY_SIZE 32

bool str_eq(const char* s1, const char* s2, int MAX_CMP) {
    if (s1 == s2) return true;
    for (u32 i = 0; i < MAX_CMP && *s1; i++) {
        if (s1[i] != s2[i])
            return false;
        if (s1[i] == '\0' && s2[i] == '\0') {
            return true;
        }
    }
    return true;
}

typedef struct {
    char* query;
    void (*callback)(const coff_t* c);
} qryEntry_t;

#define N_QUERIES (sizeof(QUERY_MAP) / sizeof(qryEntry_t))

void quit(const coff_t* coff) {
    exit(0);
}

qryEntry_t QUERY_MAP[] = {
    {.query = "strtable",   .callback = &print_str_table},
    {.query = "symtable",   .callback = &print_symbol_table},
    {.query = "relocs",     .callback = &print_all_relocs},
    {.query = "quit",       .callback = &quit},
    {.query = "cmds",       .callback = NULL},
};

void cmds(const coff_t* coff) {
    for (u32 i = 0; i < N_QUERIES; i++) {
        qryEntry_t qe = QUERY_MAP[i];
        printf("Command: %s\n", qe.query);
    }
}

int main(int nArgs, char** args) {
    if (nArgs < 2) {
        printf("Usage: coff <path-to-obj-file>\n");
        exit(1);
    }
    arena_t* arena = arena_init(MB(64));
    coff_t coff = load_coff(args[1], arena);
    /* Next goals are to understand storage classes (at least SMBL_EXT, SMBL_STAT, SMBL_FILE)*/
    bool running = true;
    char* qry = ALLOC_ARRAY(arena, char, QUERY_SIZE);
    printf("COFF v0.1 loaded successfully.\n");
    printf("Use '/cmds' to view a list of available commands.\n");
    bool foundCommand = false;
    while (running) {
        printf("$: /");
        foundCommand = false;
        fgets(qry, QUERY_SIZE - 1, stdin);
        for (u32 i = 0; i < QUERY_SIZE; i++) {
            if (qry[i] == '\n') {
                qry[i] = '\0';
            }
        }
        /* Annoying hack around the cmds <-> QUERY_MAP circular dependency*/
        if (str_eq(qry, "cmds", QUERY_SIZE)) {
            cmds(&coff);
            continue;
        }
        for (u32 i = 0; i < N_QUERIES; i++) {
            qryEntry_t qe = QUERY_MAP[i];
            if (str_eq(qry, qe.query, QUERY_SIZE)) {
                qe.callback(&coff);
                foundCommand = true;
                break;
            }
        }
        if (!foundCommand) {
            printf("Command not found: '%s'.\n", qry);
        }
    }
    arena_destroy(arena);
    return 0;
}