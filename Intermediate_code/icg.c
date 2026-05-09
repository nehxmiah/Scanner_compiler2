/*
 * icg.c
 * Quadruple Engine + Printf IC Generation + Error Reporting
 * Owner: Melanie Thuranira
 *
 * Implements:
 *   - emit()            add a quadruple to the table
 *   - newTemp()         generate fresh temp variables t1, t2, ...
 *   - newLabel()        generate fresh labels L1, L2, ...
 *   - printQuadruples() print the final quadruple table
 *   - icg_printf()      generate quadruples for printf statements
 *   - icg_error()       centralised error reporting
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "icg.h"

/* ─── GLOBAL STATE ──────────────────────────────────────────────── */
Quadruple quad_table[MAX_QUADS];
int       quad_count  = 0;
int       temp_count  = 0;
int       label_count = 0;

/* ─────────────────────────────────────────────────────────────────
 * emit(op, arg1, arg2, result)
 * Creates a new quadruple and appends it to the quad_table.
 *
 * Example call:
 *   emit("*", "y", "2", "t1")
 *   emit("if_false", "t3", "-", "L1")
 * ───────────────────────────────────────────────────────────────── */
void emit(const char *op, const char *arg1,
          const char *arg2, const char *result) {
    if (quad_count >= MAX_QUADS) {
        icg_error("Quadruple table full. Cannot emit more instructions.");
        return;
    }
    Quadruple *q = &quad_table[quad_count++];
    strncpy(q->op,     op,     MAX_ARG - 1);
    strncpy(q->arg1,   arg1,   MAX_ARG - 1);
    strncpy(q->arg2,   arg2,   MAX_ARG - 1);
    strncpy(q->result, result, MAX_ARG - 1);
}

/* ─────────────────────────────────────────────────────────────────
 * newTemp(buf)
 * Generates the next temporary variable name: t1, t2, t3, ...
 * Writes the name into buf.
 * ───────────────────────────────────────────────────────────────── */
void newTemp(char *buf) {
    snprintf(buf, MAX_ARG, "t%d", ++temp_count);
}

/* ─────────────────────────────────────────────────────────────────
 * newLabel(buf)
 * Generates the next label name: L1, L2, L3, ...
 * Writes the name into buf.
 * ───────────────────────────────────────────────────────────────── */
void newLabel(char *buf) {
    snprintf(buf, MAX_ARG, "L%d", ++label_count);
}

/* ─────────────────────────────────────────────────────────────────
 * printQuadruples()
 * Prints the complete quadruple table in a clean formatted layout.
 *
 * Output format:
 *   No.  Op          Arg1        Arg2        Result
 *   0    *           y           2           t1
 *   1    +           x           t1          t2
 * ───────────────────────────────────────────────────────────────── */
void printQuadruples(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        INTERMEDIATE CODE — QUADRUPLE REPRESENTATION          ║\n");
    printf("╠══════╦══════════════╦══════════════╦══════════════╦══════════╣\n");
    printf("║ %-4s ║ %-12s ║ %-12s ║ %-12s ║ %-8s ║\n",
           "No.", "Op", "Arg1", "Arg2", "Result");
    printf("╠══════╬══════════════╬══════════════╬══════════════╬══════════╣\n");

    for (int i = 0; i < quad_count; i++) {
        Quadruple *q = &quad_table[i];
        printf("║ %-4d ║ %-12s ║ %-12s ║ %-12s ║ %-8s ║\n",
               i, q->op, q->arg1, q->arg2, q->result);
    }

    printf("╚══════╩══════════════╩══════════════╩══════════════╩══════════╝\n");
    printf("\n");

    /* Summary */
    printf("  Total quadruples : %d\n", quad_count);
    printf("  Temp variables   : ");
    if (temp_count == 0) {
        printf("none");
    } else {
        for (int i = 1; i <= temp_count; i++) {
            printf("t%d", i);
            if (i < temp_count) printf(", ");
        }
    }
    printf("\n");

    printf("  Labels generated : ");
    if (label_count == 0) {
        printf("none");
    } else {
        for (int i = 1; i <= label_count; i++) {
            printf("L%d", i);
            if (i < label_count) printf(", ");
        }
    }
    printf("\n\n");
}

/* ─────────────────────────────────────────────────────────────────
 * icg_printf(node)
 * Generates a quadruple for a printf statement.
 *
 * MiniC printf form:  printf("format_string", id)
 *
 * Quadruple generated:
 *   (print, "format_string", id, -)
 * ───────────────────────────────────────────────────────────────── */
void icg_printf(TreeNode *node) {
    if (!node) return;

    /* Find STRING and ID children */
    TreeNode *str_node = NULL;
    TreeNode *id_node  = NULL;

    for (int i = 0; i < node->num_children; i++) {
        if (node->children[i]->symbol == STRING && !str_node)
            str_node = node->children[i];
        if (node->children[i]->symbol == ID && !id_node)
            id_node = node->children[i];
    }

    const char *str_val = str_node ? str_node->lexeme : "-";
    const char *id_val  = id_node  ? id_node->lexeme  : "-";

    /* Check variable is declared before printing */
    if (id_node) sym_lookup(id_node->lexeme);

    emit("print", str_val, id_val, "-");

    printf("[ICG] print_stmt: format=%s  var=%s\n", str_val, id_val);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_error(msg)
 * Centralised error reporting function.
 * All ICG components call this to report errors consistently.
 * ───────────────────────────────────────────────────────────────── */
void icg_error(const char *msg) {
    fprintf(stderr, "[ICG ERROR] %s\n", msg);
}