/*
 * symbol_table.c
 * Symbol Table Management + Semantic Error Detection
 * Owner: Nehemiah Koech
 *
 * Tracks all declared variables, detects undeclared variable use
 * and redeclaration errors.
 *
 * MODIFICATIONS:
 *   - Added current_scope global variable             (Q4 — Scope tracking)
 *   - Added scope_level to every sym_insert() call    (Q4 — Scope tracking)
 *   - Added sym_enter_scope() function                (Q4 — Scope tracking)
 *   - Added sym_exit_scope() function                 (Q4 — Scope tracking)
 *   - sym_print() now shows scope_level column        (Q4 — Scope tracking)
 */

#include <stdio.h>
#include <string.h>
#include "icg.h"

/* ─── GLOBALS ───────────────────────────────────────────────────── */
SymbolTable sym_table;
int current_scope = 0;   /* 0 = global (main body), 1+ = inside blocks */

/* ─────────────────────────────────────────────────────────────────
 * sym_init()
 * Initialises the symbol table to empty and resets scope to 0.
 * Called once at the start of ICG.
 * ───────────────────────────────────────────────────────────────── */
void sym_init(void) {
    sym_table.count = 0;
    current_scope   = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        sym_table.entries[i].name[0]       = '\0';
        sym_table.entries[i].type[0]       = '\0';
        sym_table.entries[i].initialised   = 0;
        sym_table.entries[i].line_declared = 0;
        sym_table.entries[i].scope_level   = 0;
    }
    printf("[SYMBOL TABLE] Initialised.\n");
}

/* ─────────────────────────────────────────────────────────────────
 * sym_enter_scope()
 * Called when entering an if or while body { }.
 * Increments the scope counter so any variables declared
 * inside the block get tagged with the current depth.
 * ───────────────────────────────────────────────────────────────── */
void sym_enter_scope(void) {
    current_scope++;
    printf("[SCOPE] Entering scope level %d\n", current_scope);
}

/* ─────────────────────────────────────────────────────────────────
 * sym_exit_scope()
 * Called when leaving an if or while body { }.
 * Removes all variables that were declared at the current scope
 * level — they are no longer visible outside the block.
 * Then decrements the scope counter.
 * ───────────────────────────────────────────────────────────────── */
void sym_exit_scope(void) {
    int i = 0;
    while (i < sym_table.count) {
        if (sym_table.entries[i].scope_level == current_scope) {
            printf("[SCOPE] Removing '%s' (declared at scope %d, now leaving scope %d)\n",
                sym_table.entries[i].name,
                sym_table.entries[i].scope_level,
                current_scope);
            /* Shift all entries above this one down by one */
            for (int j = i; j < sym_table.count - 1; j++)
                sym_table.entries[j] = sym_table.entries[j + 1];
            sym_table.count--;
            /* Do NOT increment i — check same index again after shift */
        } else {
            i++;
        }
    }
    current_scope--;
    printf("[SCOPE] Back to scope level %d\n", current_scope);
}

/* ─────────────────────────────────────────────────────────────────
 * sym_insert(name, type, line)
 * Inserts a new variable into the symbol table.
 * Tags it with the current scope_level.
 * Returns  1 on success.
 * Returns  0 if the variable is already declared (semantic error).
 * Returns -1 if the table is full.
 * ───────────────────────────────────────────────────────────────── */
int sym_insert(const char *name, const char *type, int line) {
    /* Check for redeclaration within the same scope */
    for (int i = 0; i < sym_table.count; i++) {
        if (strcmp(sym_table.entries[i].name, name) == 0 &&
            sym_table.entries[i].scope_level == current_scope) {
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

    /* Insert with current scope level */
    Symbol *s = &sym_table.entries[sym_table.count++];
    strncpy(s->name,  name, MAX_NAME - 1);
    strncpy(s->type,  type, MAX_NAME - 1);
    s->name[MAX_NAME - 1]  = '\0';
    s->type[MAX_NAME - 1]  = '\0';
    s->initialised          = 0;
    s->line_declared        = line;
    s->scope_level          = current_scope;   /* TAG WITH CURRENT SCOPE */

    printf("[SYMBOL TABLE] Inserted: %-10s  type=%-6s  line=%d  scope=%d\n",
           name, type, line, current_scope);
    return 1;
}

/* ─────────────────────────────────────────────────────────────────
 * sym_lookup(name)
 * Returns the index of the variable in the table if found.
 * Returns -1 if not found (semantic error — undeclared variable).
 * Searches from the innermost scope outward (highest scope_level first).
 * ───────────────────────────────────────────────────────────────── */
int sym_lookup(const char *name) {
    /* Search from highest scope down to 0 so inner declarations
       shadow outer ones correctly */
    for (int scope = current_scope; scope >= 0; scope--) {
        for (int i = 0; i < sym_table.count; i++) {
            if (strcmp(sym_table.entries[i].name, name) == 0 &&
                sym_table.entries[i].scope_level == scope)
                return i;
        }
    }
    /* Not found anywhere */
    fprintf(stderr,
        "[SEMANTIC ERROR] Undeclared variable '%s' used before declaration.\n",
        name);
    return -1;
}

/* ─────────────────────────────────────────────────────────────────
 * sym_update(name)
 * Marks a variable as initialised (it has been assigned a value).
 * Searches innermost scope first so it updates the right entry
 * when a variable in an inner scope shadows an outer one.
 * ───────────────────────────────────────────────────────────────── */
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
    fprintf(stderr,
        "[SEMANTIC ERROR] Cannot update undeclared variable '%s'.\n", name);
}

/* ─────────────────────────────────────────────────────────────────
 * sym_print()
 * Prints the full symbol table including scope level column.
 * ───────────────────────────────────────────────────────────────── */
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
               s->name,
               s->type,
               s->initialised ? "Yes" : "No",
               scope_label);
    }
    printf("╚══════════════╩══════════╩═══════════╩═══════════════╝\n");
    printf("  Total symbols: %d\n\n", sym_table.count);
}