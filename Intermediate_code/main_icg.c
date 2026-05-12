/*
 * main_icg.c
 * ICG Pipeline Driver
 * Owner: Melanie Thuranira
 *
 * Walks the parse tree produced by the parser and calls the
 * appropriate ICG functions for each node type.
 *
 * Pipeline:
 *   parse tree  →  ICG functions  →  quadruple table  →  print
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "icg.h"

/* Forward declarations of parser pipeline */
void parse(FILE *fp);
extern TreeNode *parse_root;  /* set by parser.c after parsing */

/* ─────────────────────────────────────────────────────────────────
 * Helper: get Nth child
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
 * icg_decl(node)
 * Handles decl_stmt → int id_list ;
 * Inserts each declared variable into the symbol table.
 * ───────────────────────────────────────────────────────────────── */
void icg_decl(TreeNode *node) {
    if (!node) return;

    /* Walk the id_list subtree collecting all declared names */
    TreeNode *id_list = findChild(node, ID_LIST);
    while (id_list) {
        /* id_list → id id_list' */
        TreeNode *id_node = findChild(id_list, ID);
        if (id_node) {
            sym_insert(id_node->lexeme, "int", 0);
        }
        /* Move to id_list' */
        TreeNode *id_list_p = findChild(id_list, ID_LIST_P);
        if (!id_list_p || id_list_p->num_children == 0) break;
        id_list = id_list_p;
    }
}

/* ─────────────────────────────────────────────────────────────────
 * icg_stmt(node)
 * Dispatches a single STMT node to the correct ICG function
 * based on what kind of statement it is.
 * ───────────────────────────────────────────────────────────────── */
void icg_stmt(TreeNode *node) {
    if (!node) return;

    /* stmt node has one child — the actual statement type */
    TreeNode *actual = child(node, 0);
    if (!actual) return;

    switch (actual->symbol) {
        case DECL_STMT:
            icg_decl(actual);
            break;

        case ASSIGN_STMT:
            icg_assign(actual);
            break;

        case IF_STMT:
            icg_if(actual);
            break;

        case WHILE_STMT:
            icg_while(actual);
            break;

        case PRINT_STMT:
            icg_printf(actual);
            break;

        default:
            /* semicolon-only statement — nothing to generate */
            break;
    }
}

/* ─────────────────────────────────────────────────────────────────
 * icg_stmt_list(node)
 * Walks stmt_list and stmt_list' nodes, calling icg_stmt
 * for each statement found.
 * ───────────────────────────────────────────────────────────────── */
void icg_stmt_list(TreeNode *node) {
    if (!node) return;

    /* stmt_list → stmt stmt_list' */
    if (node->symbol == STMT_LIST || node->symbol == STMT_LIST_P) {
        for (int i = 0; i < node->num_children; i++) {
            TreeNode *c = node->children[i];
            if (c->symbol == STMT)
                icg_stmt(c);
            else if (c->symbol == STMT_LIST_P)
                icg_stmt_list(c);
        }
    }
}

/* ─────────────────────────────────────────────────────────────────
 * run_icg(root)
 * Entry point for the ICG.
 * Walks from the PROGRAM root node down to the stmt_list.
 * ───────────────────────────────────────────────────────────────── */
void run_icg(TreeNode *root) {
    if (!root) {
        icg_error("Parse tree root is NULL. Cannot generate IC.");
        return;
    }

    printf("\n[ICG] Starting Intermediate Code Generation...\n");
    printf("─────────────────────────────────────────────\n");

    /* Initialise symbol table */
    sym_init();

    /* Find STMT_LIST inside PROGRAM */
    TreeNode *stmt_list = findChild(root, STMT_LIST);
    if (!stmt_list) {
        icg_error("No STMT_LIST found in parse tree.");
        return;
    }

    /* Walk all statements */
    icg_stmt_list(stmt_list);

    printf("─────────────────────────────────────────────\n");
    printf("[ICG] Code generation complete.\n");

    /* Print symbol table */
    sym_print();

    /* Print quadruple table */
    printQuadruples();
}

/* ═══════════════════════════════════════════════════════════════════
 * main()
 * Full pipeline:
 *   1. Run scanner  → tokens.txt
 *   2. Run parser   → parse tree
 *   3. Run ICG      → quadruples
 * ═══════════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {
    clock_t start = clock();

    const char *input_file = (argc > 1) ? argv[1] : "input.c";

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        MiniC COMPILER — Intermediate Code Generator          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Source file: %s\n\n", input_file);

    /* ── Step 1: Run scanner ── */
    printf("[1/3] Running lexical analysis...\n");
    char scan_cmd[256];
    snprintf(scan_cmd, sizeof(scan_cmd), "./scanner %s > tokens.txt", input_file);
    int scan_result = system(scan_cmd);
    if (scan_result != 0) {
        fprintf(stderr, "  Scanner failed. Compile with: gcc scanner.c -o scanner\n");
        return 1;
    }
    printf("  Tokens written to tokens.txt\n\n");

    /* ── Step 2: Run parser ── */
    printf("[2/3] Running parser...\n");
    FILE *fp = fopen("tokens.txt", "r");
    if (!fp) {
        fprintf(stderr, "  Cannot open tokens.txt\n");
        return 1;
    }
    parse(fp);
    fclose(fp);

    /* ── Step 3: Run ICG ── */
    printf("\n[3/3] Running Intermediate Code Generator...\n");

    extern TreeNode *parse_root;
    extern int syntax_error_count;

    /* Check if parsing succeeded */
    if (syntax_error_count > 0) {
        fprintf(stderr, "\n⚠️  SKIPPING CODE GENERATION\n");
        fprintf(stderr, "  Reason: %d syntax error(s) found during parsing\n", syntax_error_count);
        fprintf(stderr, "  Fix all syntax errors before generating intermediate code.\n\n");
        return 1;
    }

    if (!parse_root) {
        fprintf(stderr, "  Parse tree not available. Check parser output.\n");
        return 1;
    }

    run_icg(parse_root);

    /* Timing */
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("  Total time: %.3f seconds\n\n", elapsed);

    return 0;
}