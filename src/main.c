#include <stdio.h>
#include "coff.h"
#include "arena.h"
#include "utils.h"

#define QUERY_SIZE 32

typedef struct {
    char* query;
    void (*callback)(const coff_t* c, const char* query, arena_t* arena);
} qryEntry_t;


void quit(const coff_t* coff, const char* ignored, arena_t* _ignored) {
    exit(0);
}

static arena_t* global_arena;

static qryEntry_t QUERY_MAP[] = {
    {.query = "cmds",       .callback = NULL},
    {.query = "scns",       .callback = print_all_scns},
    {.query = "strtable",   .callback = print_str_table},
    {.query = "symtable",   .callback = print_symbol_table},
    {.query = "relocs",     .callback = print_all_relocs},
    {.query = "hexdump",    .callback = hexdump},
    {.query = "fns",        .callback = print_fns_sorted},
    {.query = "quit",       .callback = quit},
};

#define N_QUERIES (sizeof(QUERY_MAP) / sizeof(qryEntry_t))

/**
 * Don't want global_arena to be arena_t** but we also
 * can't set it where QUERY_MAP is defined so we need to
 * wait for main() to create the arena and then call this
 * function.
 */
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
    global_arena = arena_init(MB(64));
    coff_t coff = load_coff(args[1], global_arena);
    /* Next goals are to understand storage classes (at least SMBL_EXT, SMBL_STAT, SMBL_FILE)*/
    bool running = true;
    char* qry = ALLOC_ARRAY(global_arena, char, QUERY_SIZE);
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
            if (str_startswith(qry, qe.query, QUERY_SIZE)) {
                qe.callback(&coff, qry, global_arena);
                foundCommand = true;
                break;
            }
        }
        if (!foundCommand) {
            printf("Command not found: '%s'.\n", qry);
        }
    }
    arena_destroy(global_arena);
    return 0;
}