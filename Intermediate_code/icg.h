/*
 * icg.h
 * Intermediate Code Generator — Shared Definitions
 * Owner: Nehemiah Koech
 *
 * Defines all shared structs, constants, and function prototypes
 * used across the entire ICG pipeline.
 *
 * MODIFICATIONS:
 *   - Added scope_level field to Symbol struct       (Q4 — Scope tracking)
 *   - Added sym_enter_scope() / sym_exit_scope()     (Q4 — Scope tracking)
 *   - Added extern current_scope                     (Q4 — Scope tracking)
 *   - Added semantic_error_count global              (Q4 — Semantic error tracking)
 *   - Added two-pass function prototypes             (Q4 — Error detection before generation)
 */

#ifndef ICG_H
#define ICG_H

#include "../parser/parser.h"
#include "../parser/tree.h"

/* ─── CONSTANTS ─────────────────────────────────────────────────── */
#define MAX_QUADS       500
#define MAX_SYMBOLS     100
#define MAX_NAME        64
#define MAX_ARG         64
#define MAX_LABELS      100
#define MAX_TEMPS       100

/* ─── QUADRUPLE STRUCT ──────────────────────────────────────────── */
typedef struct {
    char op[MAX_ARG];
    char arg1[MAX_ARG];
    char arg2[MAX_ARG];
    char result[MAX_ARG];
} Quadruple;

/* ─── SYMBOL TABLE ENTRY ────────────────────────────────────────── */
typedef struct {
    char name[MAX_NAME];
    char type[MAX_NAME];
    int  initialised;
    int  line_declared;
    int  scope_level;   /* 0=global, 1=inside if/while, 2=nested */
} Symbol;

/* ─── SYMBOL TABLE ──────────────────────────────────────────────── */
typedef struct {
    Symbol entries[MAX_SYMBOLS];
    int    count;
} SymbolTable;

/* ─── GLOBALS ───────────────────────────────────────────────────── */
extern Quadruple quad_table[MAX_QUADS];
extern int       quad_count;
extern SymbolTable sym_table;
extern int temp_count;
extern int label_count;
extern int current_scope;           /* tracks current block depth */
extern int semantic_error_count;    /* counts semantic errors */

/* ─── PROTOTYPES ────────────────────────────────────────────────── */

/* icg.c - Core ICG functions */
void emit(const char *op, const char *arg1, const char *arg2, const char *result);
void newTemp(char *buf);
void newLabel(char *buf);
void printQuadruples(void);
void icg_printf(TreeNode *node);
void icg_error(const char *msg);

/* symbol_table.c - Symbol table management */
void sym_init(void);
int  sym_insert(const char *name, const char *type, int line);
int  sym_lookup(const char *name);
void sym_update(const char *name);
void sym_print(void);
void sym_enter_scope(void);
void sym_exit_scope(void);

/* icg_expr.c - Expression and assignment handling */
void icg_expr(TreeNode *node, char *result_out);
void icg_assign_check(TreeNode *node);      /* PASS 1: Error checking only */
void icg_assign_generate(TreeNode *node);   /* PASS 2: Code generation */

/* icg_control.c - Control flow handling */
void icg_if(TreeNode *node);
void icg_while(TreeNode *node);
void icg_condition(TreeNode *node, char *true_label, char *false_label);

/* main_icg.c - Main driver and statement handling */
void icg_decl(TreeNode *node);
void icg_stmt_check(TreeNode *node);        /* PASS 1: Error checking */
void icg_stmt_generate(TreeNode *node);     /* PASS 2: Code generation */
void icg_stmt_list_check(TreeNode *node);   /* PASS 1: Error checking */
void icg_stmt_list_generate(TreeNode *node);/* PASS 2: Code generation */
void run_icg(TreeNode *root);

#endif /* ICG_H */