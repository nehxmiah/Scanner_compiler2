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
 */

#ifndef ICG_H
#define ICG_H

#include "parser/parser.h"
#include "tree.h"

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
    int  scope_level;   /* NEW: 0=global, 1=inside if/while, 2=nested */
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
extern int current_scope;   /* NEW: tracks current block depth */

/* ─── PROTOTYPES ────────────────────────────────────────────────── */

/* icg.c */
void emit(const char *op, const char *arg1,
          const char *arg2, const char *result);
void newTemp(char *buf);
void newLabel(char *buf);
void printQuadruples(void);
void icg_printf(TreeNode *node);
void icg_error(const char *msg);

/* symbol_table.c */
void sym_init(void);
int  sym_insert(const char *name, const char *type, int line);
int  sym_lookup(const char *name);
void sym_update(const char *name);
void sym_print(void);
void sym_enter_scope(void);   /* NEW */
void sym_exit_scope(void);    /* NEW */

/* icg_expr.c */
void icg_expr(TreeNode *node, char *result_out);
void icg_assign(TreeNode *node);

/* icg_control.c */
void icg_if(TreeNode *node);
void icg_while(TreeNode *node);

/* main_icg.c */
void icg_stmt(TreeNode *node);
void icg_stmt_list(TreeNode *node);
void icg_decl(TreeNode *node);
void run_icg(TreeNode *root);

#endif /* ICG_H */