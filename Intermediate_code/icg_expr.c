/*
 * icg_expr.c
 * Expression & Assignment Intermediate Code Generation
 * Owner: Ivy Mokeira
 *
 * Generates quadruples for:
 *   - Arithmetic expressions  (+, -, *, /)
 *   - Relational expressions  (>, <, ==, !=, >=, <=)
 *   - Assignment statements   (id = exp)
 *
 * Runtime error checks:
 *   - Division by zero detection
 *   - Use of uninitialised variables
 *
 * MODIFICATIONS:
 *   - Added type mismatch check in icg_assign()      (Q4 — Type checking)
 *   - Hardened icg_factor() for ( exp ) case         (Q6 — Parentheses)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "icg.h"

/* ─────────────────────────────────────────────────────────────────
 * Helper: get the Nth child of a tree node
 * ───────────────────────────────────────────────────────────────── */
static TreeNode *child(TreeNode *node, int n) {
    if (!node || n >= node->num_children) return NULL;
    return node->children[n];
}

/* ─────────────────────────────────────────────────────────────────
 * Helper: find first child with given symbol
 * ───────────────────────────────────────────────────────────────── */
static TreeNode *findChild(TreeNode *node, int symbol) {
    if (!node) return NULL;
    for (int i = 0; i < node->num_children; i++)
        if (node->children[i]->symbol == symbol)
            return node->children[i];
    return NULL;
}

/* ─────────────────────────────────────────────────────────────────
 * runtime_check_div_zero(arg2)
 * Checks if the divisor is the literal "0".
 * If so, reports a runtime error and returns 1.
 * ───────────────────────────────────────────────────────────────── */
static int runtime_check_div_zero(const char *arg2) {
    if (strcmp(arg2, "0") == 0) {
        fprintf(stderr,
            "[RUNTIME ERROR] Division by zero detected. "
            "Cannot divide by constant 0.\n");
        return 1;
    }
    return 0;
}

/* ─────────────────────────────────────────────────────────────────
 * runtime_check_uninitialised(name)
 * Checks if a variable has been initialised before use.
 * ───────────────────────────────────────────────────────────────── */
static void runtime_check_uninitialised(const char *name) {
    int idx = sym_lookup(name);
    if (idx >= 0 && !sym_table.entries[idx].initialised) {
        fprintf(stderr,
            "[RUNTIME WARNING] Variable '%s' used before being assigned "
            "a value.\n", name);
    }
}

/* ─────────────────────────────────────────────────────────────────
 * get_operator_string(symbol)
 * Converts a terminal symbol code to its operator string.
 * ───────────────────────────────────────────────────────────────── */
static const char *get_operator_string(int symbol) {
    switch (symbol) {
        case PLUS:  return "+";
        case MINUS: return "-";
        case MUL:   return "*";
        case DIV:   return "/";
        case GT:    return ">";
        case LT:    return "<";
        case EQ:    return "==";
        case NEQ:   return "!=";
        case GTE:   return ">=";
        case LTE:   return "<=";
        default:    return "?";
    }
}

/* ─────────────────────────────────────────────────────────────────
 * icg_factor(node, result_out)
 * Handles the base case — a single id, number, or (exp).
 * Fills result_out with the name/value of the factor.
 *
 * MODIFICATION: ( exp ) case now uses LPAREN detection to find
 * the inner EXP node reliably even in nested expressions.
 * ───────────────────────────────────────────────────────────────── */
static void icg_factor(TreeNode *node, char *result_out) {
    if (!node) { strcpy(result_out, "-"); return; }

    /* factor → id */
    TreeNode *id_node = findChild(node, ID);
    if (id_node) {
        int idx = sym_lookup(id_node->lexeme);
        if (idx >= 0)
            runtime_check_uninitialised(id_node->lexeme);
        strcpy(result_out, id_node->lexeme);
        return;
    }

    /* factor → number */
    TreeNode *num_node = findChild(node, NUMBER);
    if (num_node) {
        strcpy(result_out, num_node->lexeme);
        return;
    }

    /* factor → ( exp )
     * Look for LPAREN to confirm this is a parenthesised expression.
     * Then find the EXP child and recurse into it.
     * This handles nested cases like (x + 1) * (y - 2) correctly. */
    TreeNode *lparen_node = findChild(node, LPAREN);
    if (lparen_node) {
        /* The EXP is the second child of FACTOR: FACTOR → LPAREN EXP RPAREN */
        TreeNode *inner_exp = NULL;
        for (int i = 0; i < node->num_children; i++) {
            if (node->children[i]->symbol == EXP) {
                inner_exp = node->children[i];
                break;
            }
        }
        if (inner_exp) {
            icg_expr(inner_exp, result_out);
            return;
        }
    }

    /* Fallback: try the old EXP lookup for safety */
    TreeNode *exp_node = findChild(node, EXP);
    if (exp_node) {
        icg_expr(exp_node, result_out);
        return;
    }

    strcpy(result_out, "-");
}

/* ─────────────────────────────────────────────────────────────────
 * icg_term_p(node, left, result_out)
 * Handles term' → * factor term' | / factor term' | ε
 * ───────────────────────────────────────────────────────────────── */
static void icg_term_p(TreeNode *node, const char *left, char *result_out) {
    if (!node || node->num_children == 0) {
        strcpy(result_out, left);
        return;
    }

    TreeNode *op_node     = child(node, 0);
    TreeNode *factor_node = child(node, 1);
    TreeNode *term_p_node = child(node, 2);

    char right[MAX_ARG];
    icg_factor(factor_node, right);

    /* Runtime check: division by zero */
    if (op_node && op_node->symbol == DIV) {
        if (runtime_check_div_zero(right)) {
            strcpy(result_out, "-");
            return;
        }
    }

    char temp[MAX_ARG];
    newTemp(temp);
    const char *op_str = op_node ? get_operator_string(op_node->symbol) : "?";
    emit(op_str, left, right, temp);

    icg_term_p(term_p_node, temp, result_out);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_term(node, result_out)
 * Handles term → factor term'
 * ───────────────────────────────────────────────────────────────── */
static void icg_term(TreeNode *node, char *result_out) {
    if (!node) { strcpy(result_out, "-"); return; }

    TreeNode *factor_node = child(node, 0);
    TreeNode *term_p_node = child(node, 1);

    char factor_result[MAX_ARG];
    icg_factor(factor_node, factor_result);

    icg_term_p(term_p_node, factor_result, result_out);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_exp_p(node, left, result_out)
 * Handles exp' → + term exp' | - term exp' | ε
 * ───────────────────────────────────────────────────────────────── */
static void icg_exp_p(TreeNode *node, const char *left, char *result_out) {
    if (!node || node->num_children == 0) {
        strcpy(result_out, left);
        return;
    }

    TreeNode *op_node    = child(node, 0);
    TreeNode *term_node  = child(node, 1);
    TreeNode *exp_p_node = child(node, 2);

    char right[MAX_ARG];
    icg_term(term_node, right);

    char temp[MAX_ARG];
    newTemp(temp);
    const char *op_str = op_node ? get_operator_string(op_node->symbol) : "?";
    emit(op_str, left, right, temp);

    icg_exp_p(exp_p_node, temp, result_out);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_expr(node, result_out)
 * Main expression handler.
 * Handles:
 *   - exp → term exp'            (arithmetic)
 *   - condition → exp relop exp  (relational)
 * ───────────────────────────────────────────────────────────────── */
void icg_expr(TreeNode *node, char *result_out) {
    if (!node) { strcpy(result_out, "-"); return; }

    /* CONDITION node: exp relop exp */
    if (node->symbol == CONDITION) {
        TreeNode *left_exp  = child(node, 0);
        TreeNode *relop     = child(node, 1);
        TreeNode *right_exp = child(node, 2);

        char left[MAX_ARG], right[MAX_ARG];
        icg_expr(left_exp,  left);
        icg_expr(right_exp, right);

        const char *op_str = "?";
        if (relop && relop->num_children > 0)
            op_str = get_operator_string(child(relop, 0)->symbol);

        char temp[MAX_ARG];
        newTemp(temp);
        emit(op_str, left, right, temp);
        strcpy(result_out, temp);
        return;
    }

    /* EXP node: term exp' */
    if (node->symbol == EXP) {
        TreeNode *term_node  = child(node, 0);
        TreeNode *exp_p_node = child(node, 1);

        char term_result[MAX_ARG];
        icg_term(term_node, term_result);
        icg_exp_p(exp_p_node, term_result, result_out);
        return;
    }

    /* Fallback: treat as factor */
    icg_factor(node, result_out);
}

/* ─────────────────────────────────────────────────────────────────
 * icg_assign(node)
 * Handles assign_stmt → id = exp ;
 *
 * MODIFICATION: Added type mismatch check (Q4).
 * If the RHS expression resolves to a string literal (starts with '"')
 * or another non-integer token, a SEMANTIC ERROR is reported and
 * the broken quadruple is NOT emitted.
 *
 * Quadruples generated (normal case):
 *   ... expression quadruples ...
 *   (=, result, -, id)
 * ───────────────────────────────────────────────────────────────── */
void icg_assign(TreeNode *node) {
    if (!node) return;

    /* Get the identifier (first child) */
    TreeNode *id_node  = child(node, 0);
    /* Get the expression (third child — id = exp) */
    TreeNode *exp_node = child(node, 2);

    if (!id_node || !exp_node) return;

    /* Check variable is declared */
    int idx = sym_lookup(id_node->lexeme);
    if (idx < 0) return; /* undeclared — error already reported */

    /* Generate expression quadruples */
    char exp_result[MAX_ARG];
    icg_expr(exp_node, exp_result);

    /* ── TYPE MISMATCH CHECK (Q4) ──────────────────────────────────
     * The only type in MiniC is int. Any expression result that
     * looks like a string literal is incompatible.
     * A string literal starts with a double-quote character.       */
    if (exp_result[0] == '"') {
        fprintf(stderr,
            "[SEMANTIC ERROR] Type mismatch: cannot assign string "
            "literal to int variable '%s'. Assignment ignored.\n",
            id_node->lexeme);
        return;  /* do NOT emit the broken quadruple */
    }

    /* Also catch if result is a dash (broken from upstream error) */
    if (strcmp(exp_result, "-") == 0) {
        fprintf(stderr,
            "[SEMANTIC ERROR] Type mismatch: invalid expression "
            "result for variable '%s'. Assignment ignored.\n",
            id_node->lexeme);
        return;
    }

    /* Generate assignment quadruple */
    emit("=", exp_result, "-", id_node->lexeme);

    /* Mark variable as initialised in symbol table */
    sym_update(id_node->lexeme);

    printf("[ICG] assign_stmt: %s = %s\n", id_node->lexeme, exp_result);
}