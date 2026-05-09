/*
 * icg.h
 * Intermediate Code Generator — Shared Definitions
 * Owner: Nehemiah Koech
 *
 * Defines all shared structs, constants, and function prototypes
 * used across the entire ICG pipeline.
 */

#ifndef ICG_H
#define ICG_H

#include "parser/parser.h"  /* TreeNode, Terminal, NonTerminal */
#include "tree.h"

/* ─── CONSTANTS ─────────────────────────────────────────────────── */
#define MAX_QUADS       500
#define MAX_SYMBOLS     100
#define MAX_NAME        64
#define MAX_ARG         64
#define MAX_LABELS      100
#define MAX_TEMPS       100

/* ─── QUADRUPLE STRUCT ──────────────────────────────────────────── */
/*
 * A quadruple has the form:  (op, arg1, arg2, result)
 * Example:  (*,  y,  2,  t1)
 *           (+,  x,  t1, t2)
 *           (=,  t2, -,  z )
 */
typedef struct {
    char op[MAX_ARG];       /* operator e.g. "+", "*", "=", "if_false" */
    char arg1[MAX_ARG];     /* first operand  */
    char arg2[MAX_ARG];     /* second operand ("-" if unused) */
    char result[MAX_ARG];   /* result variable or label */
} Quadruple;

/* ─── SYMBOL TABLE ENTRY ────────────────────────────────────────── */
typedef struct {
    char name[MAX_NAME];    /* variable name e.g. "x" */
    char type[MAX_NAME];    /* type e.g. "int" */
    int  initialised;       /* 1 if assigned a value, 0 otherwise */
    int  line_declared;     /* line number where declared */
} Symbol;

/* ─── SYMBOL TABLE ──────────────────────────────────────────────── */
typedef struct {
    Symbol entries[MAX_SYMBOLS];
    int    count;
} SymbolTable;

/* ─── GLOBAL QUADRUPLE TABLE ────────────────────────────────────── */
extern Quadruple quad_table[MAX_QUADS];
extern int       quad_count;

/* ─── GLOBAL SYMBOL TABLE ───────────────────────────────────────── */
extern SymbolTable sym_table;

/* ─── TEMP & LABEL COUNTERS ─────────────────────────────────────── */
extern int temp_count;
extern int label_count;

/* ═══════════════════════════════════════════════════════════════════
 *  FUNCTION PROTOTYPES
 * ═══════════════════════════════════════════════════════════════════ */

/* --- icg.c (Melanie) --- */
void emit(const char *op, const char *arg1,
          const char *arg2, const char *result);
void newTemp(char *buf);          /* fills buf with "t1", "t2", ... */
void newLabel(char *buf);         /* fills buf with "L1", "L2", ... */
void printQuadruples(void);
void icg_printf(TreeNode *node);  /* printf IC generation */
void icg_error(const char *msg);  /* centralised error reporter */

/* --- symbol_table.c (Nehemiah) --- */
void sym_init(void);
int  sym_insert(const char *name, const char *type, int line);
int  sym_lookup(const char *name);
void sym_update(const char *name);
void sym_print(void);

/* --- icg_expr.c (Ivy) --- */
void icg_expr(TreeNode *node, char *result_out);
void icg_assign(TreeNode *node);

/* --- icg_control.c (Stella) --- */
void icg_if(TreeNode *node);
void icg_while(TreeNode *node);

/* --- main_icg.c (Melanie) --- */
void icg_stmt(TreeNode *node);
void icg_stmt_list(TreeNode *node);
void icg_decl(TreeNode *node);
void run_icg(TreeNode *root);

#endif /* ICG_H */