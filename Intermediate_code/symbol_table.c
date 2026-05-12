/*
 * symbol_table.c
 * Symbol Table Management + Semantic Error Detection
 * Owner: Nehemiah Koech
 */

#include <stdio.h>
#include <string.h>
#include "icg.h"

/* ─── GLOBALS ───────────────────────────────────────────────────── */
SymbolTable sym_table;
int current_scope = 0;
int semantic_error_count = 0;

/* ───────────────────────────────────────────────────────────────── */
void sym_init(void) {
    sym_table.count = 0;
    current_scope   = 0;
    semantic_error_count = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        sym_table.entries[i].name[0]       = '\0';
        sym_table.entries[i].type[0]       = '\0';
        sym_table.entries[i].initialised   = 0;
        sym_table.entries[i].line_declared = 0;
        sym_table.entries[i].scope_level   = 0;
    }
    printf("[SYMBOL TABLE] Initialised.\n");
}

/* ───────────────────────────────────────────────────────────────── */
void sym_enter_scope(void) {
    current_scope++;
    printf("[SCOPE] Entering scope level %d\n", current_scope);
}

/* ───────────────────────────────────────────────────────────────── */
void sym_exit_scope(void) {
    int i = 0;
    while (i < sym_table.count) {
        if (sym_table.entries[i].scope_level == current_scope) {
            for (int j = i; j < sym_table.count - 1; j++)
                sym_table.entries[j] = sym_table.entries[j + 1];
            sym_table.count--;
        } else {
            i++;
        }
    }
    current_scope--;
    printf("[SCOPE] Back to scope level %d\n", current_scope);
}

/* MODIFIED: Continue even after error - don't stop */
int sym_insert(const char *name, const char *type, int line) {
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.entries[i].name, name) == 0 &&
            sym_table.entries[i].scope_level == current_scope) {
            fprintf(stderr,
                "[SEMANTIC ERROR] Line %d: Variable '%s' already declared.\n",
                line, name);
            semantic_error_count++;
            return 0;  /* Return error but DON'T exit - continue processing */
        }
    }

    if (sym_table.count >= MAX_SYMBOLS) {
        fprintf(stderr, "[SEMANTIC ERROR] Symbol table full.\n");
        semantic_error_count++;
        return -1;
    }

    Symbol *s = &sym_table.entries[sym_table.count++];
    strncpy(s->name, name, MAX_NAME - 1);
    strncpy(s->type, type, MAX_NAME - 1);
    s->name[MAX_NAME - 1] = '\0';
    s->type[MAX_NAME - 1] = '\0';
    s->initialised = 0;
    s->line_declared = line;
    s->scope_level = current_scope;

    printf("[SYMBOL TABLE] Inserted: %-10s type=%-6s line=%d scope=%d\n",
           name, type, line, current_scope);
    return 1;
}

/* MODIFIED: Continue even after error - don't stop */
int sym_lookup(const char *name) {
    for (int scope = current_scope; scope >= 0; scope--) {
        for (int i = 0; i < sym_table.count; i++) {
            if (strcmp(sym_table.entries[i].name, name) == 0 &&
                sym_table.entries[i].scope_level == scope)
                return i;
        }
    }
    fprintf(stderr, "[SEMANTIC ERROR] Undeclared variable '%s'.\n", name);
    semantic_error_count++;
    return -1;  /* Return error but DON'T exit - continue processing */
}

/* MODIFIED: Continue even after error - don't stop */
void sym_update(const char *name) {
    for (int scope = current_scope; scope >= 0; scope--) {
        for (int i = 0; i < sym_table.count; i++) {
            if (strcmp(sym_table.entries[i].name, name) == 0 &&
                sym_table.entries[i].scope_level == scope) {
                sym_table.entries[i].initialised = 1;
                return;
            }
        }
    }
    fprintf(stderr, "[SEMANTIC ERROR] Cannot update undeclared variable '%s'.\n", name);
    semantic_error_count++;
}

void sym_print(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║                   SYMBOL TABLE                       ║\n");
    printf("╠══════════════╦══════════╦═══════════╦═══════════════╣\n");
    printf("║ %-12s ║ %-8s ║ %-9s ║ %-13s ║\n",
           "Name", "Type", "Init?", "Scope Level");
    printf("╠══════════════╬══════════╬═══════════╬═══════════════╣\n");
    for (int i = 0; i < sym_table.count; i++) {
        Symbol *s = &sym_table.entries[i];
        char scope_label[32];
        if (s->scope_level == 0)
            snprintf(scope_label, sizeof(scope_label), "0 (global)");
        else
            snprintf(scope_label, sizeof(scope_label), "%d (block)", s->scope_level);
        printf("║ %-12s ║ %-8s ║ %-9s ║ %-13s ║\n",
               s->name, s->type, s->initialised ? "Yes" : "No", scope_label);
    }
    printf("╚══════════════╩══════════╩═══════════╩═══════════════╝\n");
    printf("  Total symbols: %d\n\n", sym_table.count);
}