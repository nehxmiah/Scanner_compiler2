/*
 * main.c
 * Main Driver - MiniC Compiler Pipeline
 *
 * Ties the whole pipeline together:
 *   1. Runs the scanner (scanner.c) to tokenize input.c → tokens.txt
 *   2. Opens tokens.txt and feeds it to the parser
 *   3. Parser builds and prints the parse tree
 *
 * Features:
 *   - Color-coded terminal output
 *   - Verbose mode for debugging
 *   - Skip scanner option (use existing tokens.txt)
 *   - Execution timing
 *   - Detailed error reporting
 *   - Help system
 */

#include <stdarg.h> // ← FIX: Added for va_list, va_start, va_end
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Forward declaration from parser.c
void parse(FILE *fp);

// Configuration
#define DEFAULT_INPUT_FILE "input.c"
#define TOKEN_FILE "tokens.txt"
#define SCANNER_EXEC "./scanner"
#define MAX_PATH 512

// Color codes for terminal output
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_CYAN "\033[36m"
#define COLOR_BOLD "\033[1m"
#define COLOR_DIM "\033[2m"

// Global configuration
typedef struct {
  const char *input_file;
  const char *program_name; // ← FIX: Store program name
  int verbose;
  int skip_scanner;
  int show_timing;
} Config;

static Config config = {.input_file = DEFAULT_INPUT_FILE,
                        .program_name = "./main",
                        .verbose = 0,
                        .skip_scanner = 0,
                        .show_timing = 1};

// ─────────────────────────────────────────────────────────────────────────────
// Utility Functions
// ─────────────────────────────────────────────────────────────────────────────

void printBanner() {
  printf("\n");
  printf(COLOR_BOLD COLOR_BLUE);
  printf("╔══════════════════════════════════════════════════════════════╗\n");
  printf("║           MiniC Compiler Pipeline - LL(1) Parser            ║\n");
  printf("╚══════════════════════════════════════════════════════════════╝\n");
  printf(COLOR_RESET);
  printf("\n");
}

void printHelp(const char *program_name) {
  printf("\n");
  printf(COLOR_BOLD "Usage:" COLOR_RESET "\n");
  printf("  %s [options] [input_file]\n\n", program_name);

  printf(COLOR_BOLD "Options:" COLOR_RESET "\n");
  printf("  -h, --help        Show this help message\n");
  printf("  -v, --verbose     Enable verbose output\n");
  printf("  -s, --skip-scan   Skip scanner, use existing tokens.txt\n");
  printf("  -t, --no-time     Hide timing information\n");
  printf("\n");

  printf(COLOR_BOLD "Examples:" COLOR_RESET "\n");
  printf("  %s                    # Parse input.c\n", program_name);
  printf("  %s mycode.c          # Parse mycode.c\n", program_name);
  printf("  %s -v test.c         # Verbose mode\n", program_name);
  printf("  %s -s                # Skip scanner, use existing tokens\n",
         program_name);
  printf("\n");
}

void printStep(int step, int total, const char *description) {
  printf(COLOR_BOLD COLOR_CYAN "[Step %d/%d]" COLOR_RESET " %s\n", step, total,
         description);
}

void printSuccess(const char *message, ...) {
  printf(COLOR_GREEN "  ✓ " COLOR_RESET);
  va_list args;
  va_start(args, message);
  vprintf(message, args);
  va_end(args);
  printf("\n");
}

void printError(const char *message, ...) {
  printf(COLOR_RED "  ✗ " COLOR_RESET);
  va_list args;
  va_start(args, message);
  vprintf(message, args);
  va_end(args);
  printf("\n");
}

void printWarning(const char *message, ...) {
  printf(COLOR_YELLOW "  ⚠ " COLOR_RESET);
  va_list args;
  va_start(args, message);
  vprintf(message, args);
  va_end(args);
  printf("\n");
}

void printInfo(const char *message, ...) {
  printf("    ");
  va_list args;
  va_start(args, message);
  vprintf(message, args);
  va_end(args);
  printf("\n");
}

void printVerbose(const char *message, ...) {
  if (config.verbose) {
    printf(COLOR_DIM "    [DEBUG] " COLOR_RESET);
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    printf("\n");
  }
}

void printDivider() {
  printf(
      COLOR_DIM
      "────────────────────────────────────────────────────────────" COLOR_RESET
      "\n");
}

// ─────────────────────────────────────────────────────────────────────────────
// File & System Checks
// ─────────────────────────────────────────────────────────────────────────────

int fileExists(const char *filename) { return access(filename, F_OK) == 0; }

int isExecutable(const char *filename) { return access(filename, X_OK) == 0; }

long getFileSize(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp)
    return -1;

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fclose(fp);
  return size;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pipeline Stages
// ─────────────────────────────────────────────────────────────────────────────

int runScanner(const char *input_file) {
  printf("\n");
  printStep(1, 3, "Running lexical analysis (scanner)...");

  // Check if scanner executable exists
  if (!fileExists(SCANNER_EXEC)) {
    printError("Scanner executable not found: %s", SCANNER_EXEC);
    printInfo("Compile it first: gcc scanner.c -o scanner");
    return 1;
  }

  // Check if scanner is executable
  if (!isExecutable(SCANNER_EXEC)) {
    printError("Scanner is not executable: %s", SCANNER_EXEC);
    printInfo("Fix permissions: chmod +x scanner");
    return 1;
  }

  // Check if input file exists
  if (!fileExists(input_file)) {
    printError("Input file not found: %s", input_file);
    return 1;
  }

  printVerbose("Input file: %s", input_file);
  printVerbose("Scanner: %s", SCANNER_EXEC);

  // Build command - separate stderr from token output
  char command[MAX_PATH];
  snprintf(command, sizeof(command), "%s %s > %s", SCANNER_EXEC, input_file,
           TOKEN_FILE);

  clock_t start = clock();
  int result = system(command);
  clock_t end = clock();

  double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
  printVerbose("Scanner completed in %.3f seconds", time_taken);

  if (result != 0) {
    printError("Scanner failed with exit code %d", result);
    printf("\n");
    printInfo("Troubleshooting:");
    printInfo("  1. Check scanner compilation: gcc scanner.c -o scanner");
    printInfo("  2. Verify input file exists: %s", input_file);
    printInfo("  3. Run scanner manually: %s %s", SCANNER_EXEC, input_file);
    printf("\n");
    return 1;
  }

  // Check if tokens.txt was created
  if (!fileExists(TOKEN_FILE)) {
    printError("Scanner did not create %s", TOKEN_FILE);
    return 1;
  }

  long token_size = getFileSize(TOKEN_FILE);
  printSuccess("Scanner completed (%.2f KB, %.3f sec)", token_size / 1024.0,
               time_taken);
  printVerbose("Tokens written to: %s", TOKEN_FILE);

  return 0;
}

FILE *openTokenFile(const char *token_file) {
  printf("\n");
  printStep(2, 3, "Opening token stream...");

  if (!fileExists(token_file)) {
    printError("Token file not found: %s", token_file);
    return NULL;
  }

  FILE *fp = fopen(token_file, "r");
  if (!fp) {
    printError("Could not open token file: %s", token_file);
    printInfo("Check file permissions");
    return NULL;
  }

  // Check if file is empty
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (size == 0) {
    printError("Token file is empty");
    fclose(fp);
    return NULL;
  }

  printVerbose("Token file size: %ld bytes", size);
  printSuccess("Token stream ready (%.2f KB)", size / 1024.0);

  return fp;
}

int runParser(FILE *fp) {
  printf("\n");
  printStep(3, 3, "Parsing and constructing parse tree...");
  printf("\n");

  clock_t start = clock();

  // Call the parser (includes tree building and printing)
  parse(fp);

  clock_t end = clock();
  double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

  if (config.show_timing) {
    printf("\n");
    printVerbose("Parser completed in %.3f seconds", time_taken);
  }

  return 0;
}

void printSummary(int success, double total_time) {
  printf("\n");
  printDivider();

  if (success) {
    printf(COLOR_BOLD COLOR_GREEN);
    printf("║                    PIPELINE COMPLETED SUCCESSFULLY               "
           "     ║\n");
    printf(COLOR_RESET);
  } else {
    printf(COLOR_BOLD COLOR_RED);
    printf("║                    PIPELINE COMPLETED WITH ERRORS                "
           "     ║\n");
    printf(COLOR_RESET);
  }

  printDivider();

  if (config.show_timing) {
    printf("\n");
    printf(COLOR_DIM "  Total execution time: %.3f seconds" COLOR_RESET "\n",
           total_time);
  }

  printf("\n");
  printf("Next steps:\n");
  if (success) {
    printf("  • Review the parse tree above\n");
    printf("  • Modify %s to test different code\n", config.input_file);
    printf("  • Re-run: %s [input_file]\n",
           config.program_name); // ← FIX: Use config.program_name
  } else {
    printf("  • Review error messages above\n");
    printf("  • Fix issues in %s\n", config.input_file);
    printf("  • Re-run: %s\n",
           config.program_name); // ← FIX: Use config.program_name
  }
  printf("\n");
}

// ─────────────────────────────────────────────────────────────────────────────
// Command Line Parsing
// ─────────────────────────────────────────────────────────────────────────────

void parseArguments(int argc, char *argv[]) {
  // Store program name
  config.program_name = argv[0];

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printHelp(argv[0]);
      exit(0);
    } else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--verbose") == 0) {
      config.verbose = 1;
    } else if (strcmp(argv[i], "-s") == 0 ||
               strcmp(argv[i], "--skip-scan") == 0) {
      config.skip_scanner = 1;
    } else if (strcmp(argv[i], "-t") == 0 ||
               strcmp(argv[i], "--no-time") == 0) {
      config.show_timing = 0;
    } else if (argv[i][0] != '-') {
      // Positional argument = input file
      config.input_file = argv[i];
    } else {
      printWarning("Unknown option: %s", argv[i]);
      printInfo("Use -h for help");
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Main Entry Point
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
  int exit_code = 0;
  clock_t total_start = clock();

  // Parse command line arguments
  parseArguments(argc, argv);

  // Print welcome banner
  printBanner();

  if (config.verbose) {
    printInfo("Configuration:");
    printInfo("  Input file: %s", config.input_file);
    printInfo("  Token file: %s", TOKEN_FILE);
    printInfo("  Verbose: %s", config.verbose ? "yes" : "no");
    printInfo("  Skip scanner: %s", config.skip_scanner ? "yes" : "no");
    printf("\n");
  }

  // Step 1: Run the scanner (unless skipped)
  if (!config.skip_scanner) {
    if (runScanner(config.input_file) != 0) {
      printSummary(0, 0);
      return 1;
    }
  } else {
    printf("\n");
    printStep(1, 3, "Skipping scanner (using existing tokens.txt)...");
    if (!fileExists(TOKEN_FILE)) {
      printError("tokens.txt not found. Run without -s flag first.");
      printSummary(0, 0);
      return 1;
    }
    printSuccess("Using existing token file");
  }

  // Step 2: Open the token stream
  FILE *fp = openTokenFile(TOKEN_FILE);
  if (!fp) {
    printSummary(0, 0);
    return 1;
  }

  // Step 3: Run the parser
  if (runParser(fp) != 0) {
    exit_code = 1;
  }

  // Cleanup
  fclose(fp);

  // Calculate total time
  clock_t total_end = clock();
  double total_time = ((double)(total_end - total_start)) / CLOCKS_PER_SEC;

  // Print final summary
  printSummary(exit_code == 0, total_time);

  return exit_code;
}
