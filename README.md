# 📖 README - Windows Users Guide

```markdown
# MiniC Compiler Pipeline - Windows Guide

A complete LL(1) parser implementation with lexical analysis, syntax parsing, and parse tree construction.

---

## 📋 Table of Contents

- [Prerequisites](#-prerequisites)
- [Project Structure](#-project-structure)
- [Compilation](#-compilation)
- [Running the Pipeline](#-running-the-pipeline)
- [Understanding tokens.txt](#-understanding-tokenstxt)
- [Manual Token Generation](#-manual-token-generation)
- [Running the Parser Separately](#-running-the-parser-separately)
- [Command-Line Options](#-command-line-options)
- [Troubleshooting](#-troubleshooting)

---

## 🛠️ Prerequisites

### Required Software

| Software | Purpose | Download |
|----------|---------|----------|
| **MinGW-w64** | GCC compiler for Windows | https://www.mingw-w64.org/ |
| **Git** | Version control | https://git-scm.com/download/win |

### Verify Installation

Open **Command Prompt** or **PowerShell** and run:

```cmd
gcc --version
git --version
```

If these commands work, you're ready to go! ✅

---

## 📁 Project Structure

```
scanner_compiler/
├── input.c                # Source code to parse (YOUR CODE HERE)
├── main.c                 # Main pipeline driver
├── scanner.c              # Lexical analyzer (tokenizer)
├── parser/
│   ├── parser.c           # LL(1) parser core
│   ├── parser.h           # Parser definitions
│   ├── parse_table.c      # Grammar rules & parse table
│   ├── stack.c            # Stack implementation
│   ├── stack.h            # Stack header
│   ├── token_map.c        # Token to terminal mapping
│   ├── token_map.h        # Token header
│   ├── tree.c             # Parse tree construction
│   └── tree.h             # Tree header
├── README.md              # This file
├── .gitignore             # Git ignore rules
├── scanner.exe            # Compiled scanner (after build)
├── main.exe               # Compiled pipeline (after build)
└── tokens.txt             # Generated token file (after run)
```

---

## 🔨 Compilation

### Step 1: Navigate to Project Directory

```cmd
cd C:\path\to\scanner_compiler
```

### Step 2: Compile the Scanner

```cmd
gcc scanner.c -o scanner.exe
```

### Step 3: Compile the Main Pipeline

```cmd
gcc main.c parser/parser.c parser/parse_table.c parser/stack.c parser/token_map.c parser/tree.c -I parser -o main.exe
```

### ✅ Verify Compilation

```cmd
dir *.exe
```

You should see:
```
scanner.exe
main.exe
```

---

## 🚀 Running the Pipeline

### Option 1: Run Everything at Once (Recommended)

```cmd
main.exe
```

This automatically:
1. Runs `scanner.exe` on `input.c`
2. Generates `tokens.txt`
3. Parses the tokens
4. Displays the parse tree

### Option 2: Run with Custom Input File

```cmd
main.exe mycode.c
```

### Option 3: Run with Verbose Mode

```cmd
main.exe -v input.c
```

### Option 4: Show Help

```cmd
main.exe --help
```

---

## 📄 Understanding tokens.txt

### What is tokens.txt?

`tokens.txt` is an **intermediate file** that contains the tokenized output from the scanner. It bridges the gap between the **scanner** (lexical analysis) and the **parser** (syntax analysis).

### How is it Generated?

```
┌─────────────┐     scanner.exe     ┌─────────────┐
│   input.c   │ ──────────────────▶ │ tokens.txt  │
│ (source code)│                     │  (tokens)   │
└─────────────┘                     └─────────────┘
```

### File Format

```
LINE   LEXEME                         TOKEN_NAME                 CATEGORY
---------------------------------------------------------------------------
1      int                            KEYWORD_INT                KEYWORD
1      main                           KEYWORD_MAIN               KEYWORD
1      (                              LEFT_PARENTHESIS           PUNCTUATOR
1      )                              RIGHT_PARENTHESIS          PUNCTUATOR
1      {                              LEFT_BRACKETS              PUNCTUATOR
2      int                            KEYWORD_INT                KEYWORD
2      x                              IDENTIFIER                 IDENTIFIER
2      ,                              COMMA                      PUNCTUATOR
2      y                              IDENTIFIER                 IDENTIFIER
2      ;                              SEMICOLON                  PUNCTUATOR
...
```

### Why Use tokens.txt?

| Benefit | Explanation |
|---------|-------------|
| **Separation of concerns** | Scanner and parser are independent modules |
| **Debugging** | You can inspect tokens before parsing |
| **Reusability** | Parser can run multiple times on same tokens |
| **Performance** | Skip scanning if tokens haven't changed |

---

## 🔧 Manual Token Generation

### When to Generate Manually

- You want to inspect tokens before parsing
- You want to skip the scanner step (use `-s` flag)
- You're debugging lexical analysis
- You want to cache tokens for multiple parser runs

### How to Generate tokens.txt Manually

```cmd
scanner.exe input.c > tokens.txt
```

### Verify Token File

```cmd
type tokens.txt
```

Or view in a text editor:
```cmd
notepad tokens.txt
```

### Check File Size

```cmd
dir tokens.txt
```

---

## 🧠 Running the Parser Separately

### When to Run Parser Alone

- You already have `tokens.txt` from a previous run
- You want to test different parser configurations
- You're debugging syntax analysis only

### How to Run Parser Separately

**Note:** The current `main.exe` handles both scanner and parser together. To run parser only:

```cmd
main.exe -s
```

The `-s` (skip-scan) flag tells the pipeline to use existing `tokens.txt` without running the scanner.

### Full Workflow Example

```cmd
# Step 1: Generate tokens (first time)
scanner.exe input.c > tokens.txt

# Step 2: Run parser (uses existing tokens.txt)
main.exe -s

# Step 3: Modify input.c, regenerate tokens
scanner.exe input.c > tokens.txt

# Step 4: Run parser again
main.exe -s
```

---

## 🎛️ Command-Line Options

| Flag | Description | Example |
|------|-------------|---------|
| `-h`, `--help` | Show help message | `main.exe --help` |
| `-v`, `--verbose` | Enable debug output | `main.exe -v input.c` |
| `-s`, `--skip-scan` | Skip scanner, use existing tokens.txt | `main.exe -s` |
| `-t`, `--no-time` | Hide timing information | `main.exe -t` |
| *(none)* | Parse default `input.c` | `main.exe` |
| `filename.c` | Parse custom input file | `main.exe mycode.c` |

---

## ❌ Troubleshooting

### Error: `gcc is not recognized`

**Cause:** MinGW not installed or not in PATH

**Fix:**
1. Install MinGW-w64 from https://www.mingw-w64.org/
2. Add `C:\Program Files\mingw-w64\bin` to system PATH
3. Restart Command Prompt

### Error: `scanner.exe is not recognized`

**Cause:** Scanner not compiled

**Fix:**
```cmd
gcc scanner.c -o scanner.exe
```

### Error: `Cannot open file: input.c`

**Cause:** Input file doesn't exist in current directory

**Fix:**
```cmd
dir input.c
# If not found, create it or specify full path
main.exe C:\path\to\input.c
```

### Error: `tokens.txt not found`

**Cause:** Scanner didn't run or failed

**Fix:**
```cmd
# Run scanner manually
scanner.exe input.c > tokens.txt

# Verify it was created
dir tokens.txt

# Then run parser
main.exe -s
```

### Error: `Permission denied`

**Cause:** File permissions or antivirus blocking

**Fix:**
```cmd
# Run as Administrator
# Or disable antivirus temporarily
# Or move project to a different folder (not Program Files)
```

### Error: Parse tree shows errors

**Cause:** Syntax errors in `input.c`

**Fix:**
1. Review error messages from parser
2. Check `input.c` for missing `;`, `)`, `}`
3. Ensure all strings use underscores for spaces: `"hello_world"` instead of `"hello world"`

---

## 📊 Complete Workflow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    WINDOWS COMPILER PIPELINE                     │
└─────────────────────────────────────────────────────────────────┘

                    ┌──────────────┐
                    │   input.c    │  (Your C-like source code)
                    └──────┬───────┘
                           │
                           ▼
              ┌────────────────────────┐
              │   scanner.exe          │
              │   (Lexical Analysis)   │
              │                        │
              │  - Reads characters    │
              │  - Identifies tokens   │
              │  - Handles errors      │
              └───────────┬────────────┘
                          │
                          │ Writes to stdout
                          ▼
              ┌────────────────────────┐
              │   tokens.txt           │  (Intermediate file)
              │                        │
              │  LINE  LEXEME  TOKEN   │
              │  1     int     KEYWORD │
              │  ...   ...     ...     │
              └───────────┬────────────┘
                          │
                          │ Read by parser
                          ▼
              ┌────────────────────────┐
              │   main.exe             │
              │   (Syntax Analysis)    │
              │                        │
              │  - LL(1) parsing       │
              │  - Tree construction   │
              │  - Error recovery      │
              └───────────┬────────────┘
                          │
                          │ Displays to console
                          ▼
              ┌────────────────────────┐
              │   Parse Tree Output    │
              │                        │
              │  ├── PROGRAM           │
              │  │   ├── INT = "int"   │
              │  │   ├── MAIN = "main" │
              │  │   └── ...           │
              └────────────────────────┘
```

---

## 🧪 Example Session

```cmd
C:\scanner_compiler> dir
 input.c
 main.c
 scanner.c
 parser\

C:\scanner_compiler> gcc scanner.c -o scanner.exe

C:\scanner_compiler> gcc main.c parser\parser.c parser\parse_table.c parser\stack.c parser\token_map.c parser\tree.c -I parser -o main.exe

C:\scanner_compiler> main.exe

╔══════════════════════════════════════════════════════════════╗
║           MiniC Compiler Pipeline - LL(1) Parser            ║
╚══════════════════════════════════════════════════════════════╝

[Step 1/3] Running lexical analysis (scanner)...
  ✓ Scanner completed (1.25 KB, 0.003 sec)

[Step 2/3] Opening token stream...
  ✓ Token stream ready (1.25 KB)

[Step 3/3] Parsing and constructing parse tree...

--- Beginning LL(1) Parse with Error Reporting ---
[001] EXPAND: PROGRAM -> INT MAIN LPAREN RPAREN LBRACE STMT_LIST RBRACE
[002] MATCH: Expected 'INT', found 'int'.
...

========== PARSE TREE ==========
├── PROGRAM
│   ├── INT = "int"
│   ├── MAIN = "main"
...
================================

────────────────────────────────────────────────────────────
║                    PIPELINE COMPLETED SUCCESSFULLY                    ║
────────────────────────────────────────────────────────────

  Total execution time: 0.015 seconds
```

---

## 📝 Sample input.c

```c
int main() {
  int x, y, z;
  x = 10;
  y = 5;
  z = x + y * 2;

  if (z > 20) {
    printf("z_is_greater_than_20:%d", z);
  }

  while (x > 0) {
    x = x - 1;
  }
}
```

**Note:** String literals with spaces are automatically converted to underscores by the scanner (e.g., `"hello world"` becomes `"hello_world"` in tokens).

---

## 🎯 Quick Reference Card

```cmd
# Compile everything
gcc scanner.c -o scanner.exe
gcc main.c parser\parser.c parser\parse_table.c parser\stack.c parser\token_map.c parser\tree.c -I parser -o main.exe

# Run full pipeline
main.exe

# Run with custom file
main.exe mycode.c

# Skip scanner (use existing tokens.txt)
main.exe -s

# Verbose mode
main.exe -v input.c

# Show help
main.exe --help

# Generate tokens manually
scanner.exe input.c > tokens.txt

# View tokens
type tokens.txt

# Clean build artifacts
del scanner.exe main.exe tokens.txt
```

---

## 📞 Support

If you encounter issues:

1. Check error messages carefully
2. Verify all files exist in correct locations
3. Ensure MinGW is properly installed
4. Review `input.c` for syntax errors
5. Run with `-v` flag for debug output

---

## 📄 License

This project is for educational purposes.

---

**Happy Parsing!** 🎉
```

---

## 📁 Save This as `README_WINDOWS.md`

```bash
# In your project directory (WSL or Git Bash on Windows)
cat > README_WINDOWS.md << 'EOF'
[paste the markdown content above]
EOF

# Add and commit
git add README_WINDOWS.md
git commit -m "Add Windows-specific README with compilation and usage guide"
git push origin main
```

This README provides Windows users with everything they need to compile, run, and understand the token generation process! 🚀
