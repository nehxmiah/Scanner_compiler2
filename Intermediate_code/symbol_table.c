/*
 * symbol_table.c
 * Symbol Table Management + Semantic Error Detection
 * Owner: Nehemiah Koech
 *
 * Tracks all declared variables, detects undeclared variable use
 * and redeclaration errors.
 */

#include <stdio.h>
#include <string.h>
#include "icg.h"

/* Global symbol table instance */
SymbolTable sym_table;

/* ─────────────────────────────────────────────────────────────────
 * sym_init()
 * Initialises the symbol table to empty.
 * Called once at the start of ICG.
 * ───────────────────────────────────────────────────────────────── */
void sym_init(void) {
    sym_table.count = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        sym_table.entries[i].name[0]    = '\0';
        sym_table.entries[i].type[0]    = '\0';
        sym_table.entries[i].initialised = 0;
        sym_table.entries[i].line_declared = 0;
    }
    printf("[SYMBOL TABLE] Initialised.\n");
}

/* ─────────────────────────────────────────────────────────────────
 * sym_insert(name, type, line)
 * Inserts a new variable into the symbol table.
 * Returns  1 on success.
 * Returns  0 if the variable is already declared (semantic error).
 * Returns -1 if the table is full.
 * ───────────────────────────────────────────────────────────────── */
int sym_insert(const char *name, const char *type, int line) {
    /* Check for redeclaration */
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.entries[i].name, name) == 0) {
            fprintf(stderr,
                "[SEMANTIC ERROR] Line %d: Variable '%s' already declared "
                "(first declared on line %d).\n",
                line, name, sym_table.entries[i].line_declared);
            return 0;
        }
    }

    /* Check capacity */
    if (sym_table.count >= MAX_SYMBOLS) {
        fprintf(stderr,
            "[SEMANTIC ERROR] Symbol table full. Cannot insert '%s'.\n", name);
        return -1;
    }

    /* Insert */
    Symbol *s = &sym_table.entries[sym_table.count++];
    strncpy(s->name,  name, MAX_NAME - 1);
    strncpy(s->type,  type, MAX_NAME - 1);
    s->initialised   = 0;
    s->line_declared = line;

    printf("[SYMBOL TABLE] Inserted: %-10s  type=%-6s  line=%d\n",
           name, type, line);
    return 1;
}

/* ─────────────────────────────────────────────────────────────────
 * sym_lookup(name)
 * Returns the index of the variable in the table if found.
 * Returns -1 if not found (semantic error — undeclared variable).
 * ───────────────────────────────────────────────────────────────── */
int sym_lookup(const char *name) {
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.entries[i].name, name) == 0)
            return i;
    }
    /* Not found — undeclared variable */
    fprintf(stderr,
        "[SEMANTIC ERROR] Undeclared variable '%s' used before declaration.\n",
        name);
    return -1;
}

/* ─────────────────────────────────────────────────────────────────
 * sym_update(name)
 * Marks a variable as initialised (i.e., it has been assigned).
 * Called whenever an assignment quadruple is generated for name.
 * ───────────────────────────────────────────────────────────────── */
void sym_update(const char *name) {
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.entries[i].name, name) == 0) {
            sym_table.entries[i].initialised = 1;
            return;
        }
    }
    /* If not found, report error */
    fprintf(stderr,
        "[SEMANTIC ERROR] Cannot update undeclared variable '%s'.\n", name);
}

/* ─────────────────────────────────────────────────────────────────
 * sym_print()
 * Prints the full symbol table — useful for debugging and report.
 * ───────────────────────────────────────────────────────────────── */
void sym_print(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║              SYMBOL TABLE                    ║\n");
    printf("╠══════════════╦══════════╦════════════════════╣\n");
    printf("║ %-12s ║ %-8s ║ %-19s ║\n",
           "Name", "Type", "Initialised");
    printf("╠══════════════╬══════════╬═══════════════════╣\n");
    for (int i = 0; i < sym_table.count; i++) {
        Symbol *s = &sym_table.entries[i];
        printf("║ %-12s ║ %-8s ║ %-19s ║\n",
               s->name,
               s->type,
               s->initialised ? "Yes" : "No");
    }
    printf("╚══════════════╩══════════╩═══════════════════╝\n");
    printf("  Total symbols: %d\n\n", sym_table.count);
}