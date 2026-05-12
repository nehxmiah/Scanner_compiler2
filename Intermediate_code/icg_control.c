/*
 * icg_control.c
 * Control Flow Intermediate Code Generation
 * Owner: Stella Njambi
 *
 * Generates quadruples for:
 *   - if statements  (conditional jumps)
 *   - while loops    (loop labels + back-jumps)
 *
 * Structural error detection:
 *   - Missing conditions
 *   - Infinite loop detection (while with constant true condition)
 *
 * MODIFICATIONS:
 *   - Added sym_enter_scope() / sym_exit_scope() around
 *     if and while bodies                              (Q4 — Scope tracking)
 *   - Variables declared inside if/while bodies are now
 *     removed from the symbol table when the block ends.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "icg.h"

/* Forward declarations */
extern void icg_stmt_list(TreeNode *node);
extern void icg_expr(TreeNode *node, char *result_out);

/* ─────────────────────────────────────────────────────────────────
 * Helper: get the Nth child of a tree node
 * ───────────────────────────────────────────────────────────────── */
static TreeNode *child(TreeNode *node, int n) {
    if (!node || n >= node->num_children) return NULL;
    return node->children[n];
}

/* ─────────────────────────────────────────────────────────────────
 * Helper: find child node with a given symbol
 * ───────────────────────────────────────────────────────────────── */
static TreeNode *findChild(TreeNode *node, int symbol) {
    if (!node) return NULL;
    for (int i = 0; i < node->num_children; i++) {
        if (node->children[i]->symbol == symbol)
            return node->children[i];
    }
    return NULL;
}

/* ─────────────────────────────────────────────────────────────────
 * structural_check_condition(cond_node, construct)
 * Checks for:
 *   - Missing condition (null node)
 *   - Constant-only condition in while → infinite loop warning
 * Returns 1 if valid, 0 if structural error.
 * ───────────────────────────────────────────────────────────────── */
static int structural_check_condition(TreeNode *cond_node,
                                      const char *construct) {
    if (!cond_node) {
        fprintf(stderr,
            "[STRUCTURAL ERROR] %s statement has missing condition.\n",
            construct);
        return 0;
    }

    /* Check for constant-only condition (infinite loop risk).
     * condition → exp relop exp
     * If both sides are NUMBER-only factors, warn. */
    if (cond_node->num_children >= 3) {
        TreeNode *left_exp  = child(cond_node, 0);
        TreeNode *right_exp = child(cond_node, 2);

        int left_is_const  = 0;
        int right_is_const = 0;

        if (left_exp && left_exp->num_children > 0) {
            TreeNode *term = child(left_exp, 0);
            if (term && term->num_children > 0) {
                TreeNode *factor = child(term, 0);
                if (factor && factor->num_children > 0) {
                    if (child(factor, 0) &&
                        child(factor, 0)->symbol == NUMBER)
                        left_is_const = 1;
                }
            }
        }

        if (right_exp && right_exp->num_children > 0) {
            TreeNode *term = child(right_exp, 0);
            if (term && term->num_children > 0) {
                TreeNode *factor = child(term, 0);
                if (factor && factor->num_children > 0) {
                    if (child(factor, 0) &&
                        child(factor, 0)->symbol == NUMBER)
                        right_is_const = 1;
                }
            }
        }

        if (left_is_const && right_is_const &&
            strcmp(construct, "while") == 0) {
            fprintf(stderr,
                "[STRUCTURAL WARNING] while loop has constant condition "
                "— possible infinite loop.\n");
        }
    }

    return 1;
}

/* ─────────────────────────────────────────────────────────────────
 * icg_if(node)
 *
 * Parse tree structure:
 *   IF_STMT → IF ( CONDITION ) { STMT_LIST }
 *
 * Quadruples generated:
 *   (op,       left, right, t1)   ← evaluate condition
 *   (if_false, t1,   -,     L1)   ← skip body if false
 *   ... body quadruples ...
 *   (label,    -,    -,     L1)   ← end of if block
 *
 * MODIFICATION: sym_enter_scope() called before body,
 * sym_exit_scope() called after body so variables declared
 * inside the if block are cleaned up properly.
 * ───────────────────────────────────────────────────────────────── */
void icg_if(TreeNode *node) {
    if (!node) return;

    TreeNode *cond_node      = findChild(node, CONDITION);
    TreeNode *stmt_list_node = findChild(node, STMT_LIST);

    /* Structural check */
    if (!structural_check_condition(cond_node, "if")) return;

    /* Evaluate the condition */
    char cond_temp[MAX_ARG];
    icg_expr(cond_node, cond_temp);

    /* Create end label */
    char L_end[MAX_ARG];
    newLabel(L_end);

    /* Jump to end if condition is false */
    emit("if_false", cond_temp, "-", L_end);

    /* Enter new scope for the if body */
    sym_enter_scope();

    /* Generate body quadruples */
    if (stmt_list_node)
        icg_stmt_list(stmt_list_node);

    /* Exit if body scope — removes variables declared inside */
    sym_exit_scope();

    /* Place the end label */
    emit("label", "-", "-", L_end);

    printf("[ICG] if_stmt: condition=%s  end_label=%s\n",
           cond_temp, L_end);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_while(node)
 *
 * Parse tree structure:
 *   WHILE_STMT → WHILE ( CONDITION ) { STMT_LIST }
 *
 * Quadruples generated:
 *   (label,    -,    -,     L_start) ← top of loop
 *   (op,       left, right, t1)      ← evaluate condition
 *   (if_false, t1,   -,     L_end)   ← exit if false
 *   ... body quadruples ...
 *   (goto,     -,    -,     L_start) ← loop back
 *   (label,    -,    -,     L_end)   ← exit label
 *
 * MODIFICATION: sym_enter_scope() called before body,
 * sym_exit_scope() called after body so variables declared
 * inside the while block are cleaned up properly.
 * ───────────────────────────────────────────────────────────────── */
void icg_while(TreeNode *node) {
    if (!node) return;

    TreeNode *cond_node      = findChild(node, CONDITION);
    TreeNode *stmt_list_node = findChild(node, STMT_LIST);

    /* Structural check */
    if (!structural_check_condition(cond_node, "while")) return;

    /* Generate start and end labels */
    char L_start[MAX_ARG];
    char L_end[MAX_ARG];
    newLabel(L_start);
    newLabel(L_end);

    /* Place loop start label */
    emit("label", "-", "-", L_start);

    /* Evaluate condition */
    char cond_temp[MAX_ARG];
    icg_expr(cond_node, cond_temp);

    /* Exit loop if condition is false */
    emit("if_false", cond_temp, "-", L_end);

    /* Enter new scope for the while body */
    sym_enter_scope();

    /* Generate body quadruples */
    if (stmt_list_node)
        icg_stmt_list(stmt_list_node);

    /* Exit while body scope — removes variables declared inside */
    sym_exit_scope();

    /* Jump back to loop start */
    emit("goto", "-", "-", L_start);

    /* Place exit label */
    emit("label", "-", "-", L_end);

    printf("[ICG] while_stmt: condition=%s  start=%s  end=%s\n",
           cond_temp, L_start, L_end);
}