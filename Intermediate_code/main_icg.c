/*
 * main_icg.c
 * ICG Pipeline Driver
 * Owner: Melanie Thuranira
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "icg.h"

void parse(FILE *fp);
extern TreeNode *parse_root;
extern int semantic_error_count;

/* For use by icg_control.c */
void icg_stmt_list_generate(TreeNode *node);
void icg_stmt_generate(TreeNode *node);

static TreeNode *child(TreeNode *node, int n) {
    if (!node || n >= node->num_children) return NULL;
    return node->children[n];
}

static TreeNode *findChild(TreeNode *node, int symbol) {
    if (!node) return NULL;
    for (int i = 0; i < node->num_children; i++)
        if (node->children[i]->symbol == symbol)
            return node->children[i];
    return NULL;
}

/* Forward declarations */
void icg_assign_check(TreeNode *node);
void icg_assign_generate(TreeNode *node);
void icg_if(TreeNode *node);
void icg_while(TreeNode *node);
void icg_printf(TreeNode *node);

/* ─────────────────────────────────────────────────────────────────
 * icg_decl(node)
 * Handles decl_stmt -> int id_list ;
 * ───────────────────────────────────────────────────────────────── */
void icg_decl(TreeNode *node) {
    if (!node) return;
    
    TreeNode *id_list = findChild(node, ID_LIST);
    while (id_list) {
        TreeNode *id_node = findChild(id_list, ID);
        if (id_node) {
            sym_insert(id_node->lexeme, "int", 0);
        }
        TreeNode *id_list_p = findChild(id_list, ID_LIST_P);
        if (!id_list_p || id_list_p->num_children == 0) break;
        id_list = id_list_p;
    }
}

/* ─────────────────────────────────────────────────────────────────
 * PASS 1: SCAN FOR ERRORS
 * ───────────────────────────────────────────────────────────────── */
void icg_stmt_check(TreeNode *node) {
    if (!node) return;

    TreeNode *actual = child(node, 0);
    if (!actual) return;

    switch (actual->symbol) {
        case DECL_STMT:
            icg_decl(actual);
            break;
        case ASSIGN_STMT:
            icg_assign_check(actual);
            break;
        case IF_STMT:
        case WHILE_STMT:
        case PRINT_STMT:
            break;
        default:
            break;
    }
}

void icg_stmt_list_check(TreeNode *node) {
    if (!node) return;

    if (node->symbol == STMT_LIST || node->symbol == STMT_LIST_P) {
        for (int i = 0; i < node->num_children; i++) {
            TreeNode *c = node->children[i];
            if (c->symbol == STMT)
                icg_stmt_check(c);
            else if (c->symbol == STMT_LIST_P)
                icg_stmt_list_check(c);
        }
    }
}

/* ─────────────────────────────────────────────────────────────────
 * PASS 2: GENERATE ICG (NON-STATIC - called by icg_control.c)
 * ───────────────────────────────────────────────────────────────── */
void icg_stmt_generate(TreeNode *node) {
    if (!node) return;

    TreeNode *actual = child(node, 0);
    if (!actual) return;

    switch (actual->symbol) {
        case DECL_STMT:
            break;
        case ASSIGN_STMT:
            icg_assign_generate(actual);
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
            break;
    }
}

void icg_stmt_list_generate(TreeNode *node) {
    if (!node) return;

    if (node->symbol == STMT_LIST || node->symbol == STMT_LIST_P) {
        for (int i = 0; i < node->num_children; i++) {
            TreeNode *c = node->children[i];
            if (c->symbol == STMT)
                icg_stmt_generate(c);
            else if (c->symbol == STMT_LIST_P)
                icg_stmt_list_generate(c);
        }
    }
}

/* ─────────────────────────────────────────────────────────────────
 * run_icg(root)
 * ───────────────────────────────────────────────────────────────── */
void run_icg(TreeNode *root) {
    if (!root) {
        icg_error("Parse tree root is NULL.");
        return;
    }

    printf("\n[ICG] Starting Intermediate Code Generation...\n");
    printf("─────────────────────────────────────────────\n");

    sym_init();
    quad_count = 0;
    temp_count = 0;
    semantic_error_count = 0;

    TreeNode *stmt_list = findChild(root, STMT_LIST);
    if (!stmt_list) {
        icg_error("No STMT_LIST found.");
        return;
    }

    printf("\n[SCANNING] Checking for semantic errors...\n");
    icg_stmt_list_check(stmt_list);
    
    printf("\n[SCANNING] Complete. Found %d semantic error(s).\n", semantic_error_count);
    printf("─────────────────────────────────────────────\n");

    if (semantic_error_count > 0) {
        printf("\n╔══════════════════════════════════════════════════════════════╗\n");
        printf("║     ⚠️  SEMANTIC ERRORS DETECTED - ICG OUTPUT SUPPRESSED  ⚠️   ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        printf("  %d semantic error(s) found.\n", semantic_error_count);
        printf("  No intermediate code generated.\n");
        printf("  Fix all semantic errors and recompile.\n\n");
        return;
    }

    printf("\n[GENERATING] No errors found. Creating intermediate code...\n");
    
    quad_count = 0;
    temp_count = 0;
    
    icg_stmt_list_generate(stmt_list);

    printf("─────────────────────────────────────────────\n");
    printf("[ICG] Code generation complete.\n");

    printf("\n✅ SUCCESS: No semantic errors. Generating output...\n\n");
    sym_print();
    printQuadruples();
}

/* ─────────────────────────────────────────────────────────────────
 * main()
 * ───────────────────────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    clock_t start = clock();

    const char *input_file = (argc > 1) ? argv[1] : "input.c";

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        MiniC COMPILER — Intermediate Code Generator          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Source file: %s\n\n", input_file);

    printf("[1/3] Running lexical analysis...\n");
    char scan_cmd[256];
    snprintf(scan_cmd, sizeof(scan_cmd), "./scanner %s > tokens.txt", input_file);
    int scan_result = system(scan_cmd);
    if (scan_result != 0) {
        fprintf(stderr, "  Scanner failed.\n");
        return 1;
    }
    printf("  Tokens written to tokens.txt\n\n");

    printf("[2/3] Running parser...\n");
    FILE *fp = fopen("tokens.txt", "r");
    if (!fp) {
        fprintf(stderr, "  Cannot open tokens.txt\n");
        return 1;
    }
    parse(fp);
    fclose(fp);

    printf("\n[3/3] Running Intermediate Code Generator...\n");

    extern TreeNode *parse_root;
    extern int syntax_error_count;

    if (syntax_error_count > 0) {
        fprintf(stderr, "\n⚠️  SKIPPING CODE GENERATION\n");
        fprintf(stderr, "  %d syntax error(s) found.\n\n", syntax_error_count);
        return 1;
    }

    if (!parse_root) {
        fprintf(stderr, "  Parse tree not available.\n");
        return 1;
    }

    run_icg(parse_root);

    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("  Total time: %.3f seconds\n\n", elapsed);

    return (semantic_error_count > 0) ? 1 : 0;
}