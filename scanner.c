#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 500
#define MAX_LINE 1000

// Keywords
char *keywords[] = {"int", "if", "while", "printf", "return", "main"};
char *keyword_tokens[] = {"KEYWORD_INT",    "KEYWORD_IF",     "KEYWORD_WHILE",
                          "KEYWORD_PRINTF", "KEYWORD_RETURN", "KEYWORD_MAIN"};

int num_keywords = 6;

// Check keyword
char *getKeywordToken(char *str) {
  for (int i = 0; i < num_keywords; i++) {
    if (strcmp(str, keywords[i]) == 0)
      return keyword_tokens[i];
  }
  return NULL;
}

// Print token helper
void printToken(int line, char *lexeme, char *token, char *category) {
  printf("%-6d %-30s %-25s %-15s\n", line, lexeme, token, category);
}

// Print lexical error with detailed feedback
void printLexicalError(int line, char *lexeme, char *message) {
  fprintf(stderr, "[SCANNER ERROR] Line %d: %s (found: '%s')\n", line, message,
          lexeme);
  printToken(line, lexeme, "LEXICAL_ERROR", "INVALID");
}

int main(int argc, char *argv[]) {
  FILE *fp;
  char ch, buffer[MAX];
  char line_buffer[MAX_LINE];
  int i = 0, line = 1;
  int error_count = 0;
  int token_count = 0;

  // Allow custom input file via command line, default to input.c
  const char *input_file = "input.c";
  if (argc > 1) {
    input_file = argv[1];
  }

  fp = fopen(input_file, "r");

  if (fp == NULL) {
    fprintf(stderr, "[SCANNER ERROR] Cannot open file: %s\n", input_file);
    fprintf(stderr, "Usage: %s [input_file.c]\n", argv[0]);
    return 1;
  }

  printf("\n%-6s %-30s %-25s %-15s\n", "LINE", "LEXEME", "TOKEN_NAME",
         "CATEGORY");
  printf("---------------------------------------------------------------------"
         "---------------------\n");

  while ((ch = fgetc(fp)) != EOF) {

    // Track line numbers
    if (ch == '\n') {
      line++;
      continue;
    }

    // Ignore whitespace (space, tab, newline, carriage return)
    if (isspace(ch))
      continue;

    // Skip single-line comments (//)
    if (ch == '/') {
      char next = fgetc(fp);
      if (next == '/') {
        // Skip until end of line
        while ((ch = fgetc(fp)) != EOF && ch != '\n')
          ;
        if (ch == '\n') {
          line++;
        }
        continue;
      } else if (next == '*') {
        // Multi-line comment /* */
        int comment_line = line;
        char prev = '\0';
        while ((ch = fgetc(fp)) != EOF) {
          if (ch == '\n')
            line++;
          if (prev == '*' && ch == '/')
            break;
          prev = ch;
        }
        if (ch == EOF) {
          fprintf(stderr,
                  "[SCANNER ERROR] Line %d: Unterminated multi-line comment\n",
                  comment_line);
          error_count++;
        }
        continue;
      } else {
        // Division operator
        printToken(line, "/", "OPERATOR_DIVISION", "OPERATOR");
        token_count++;
        fseek(fp, -1, SEEK_CUR);
        continue;
      }
    }

    // IDENTIFIER / KEYWORD (MAXIMAL MUNCH)
    if (isalpha(ch) || ch == '_') {
      i = 0;
      buffer[i++] = ch;

      while ((ch = fgetc(fp)) != EOF && (isalnum(ch) || ch == '_'))
        buffer[i++] = ch;

      buffer[i] = '\0';

      char *token = getKeywordToken(buffer);

      if (token) {
        printToken(line, buffer, token, "KEYWORD");
      } else {
        printToken(line, buffer, "IDENTIFIER", "IDENTIFIER");
      }
      token_count++;

      if (ch != EOF) {
        fseek(fp, -1, SEEK_CUR);
      }
    }

    // NUMBERS (Integer literals)
    else if (isdigit(ch)) {
      i = 0;
      buffer[i++] = ch;

      while ((ch = fgetc(fp)) != EOF && isdigit(ch))
        buffer[i++] = ch;

      buffer[i] = '\0';

      // Check for invalid number format (e.g., number followed by letter)
      if (ch != EOF && (isalpha(ch) || ch == '_')) {
        // Invalid: number followed by identifier characters
        while ((ch = fgetc(fp)) != EOF && (isalnum(ch) || ch == '_'))
          buffer[i++] = ch;
        buffer[i] = '\0';
        printLexicalError(line, buffer,
                          "Invalid number format (letters after digits)");
        error_count++;
      } else {
        printToken(line, buffer, "INTEGER_LITERAL", "NUMBER");
        token_count++;
        if (ch != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    // STRING LITERAL - WITH SPACE HANDLING FIX
    else if (ch == '"') {
      int j = 0;
      buffer[j++] = '"';
      int start_line = line;
      int valid = 0;
      int escape = 0;

      while ((ch = fgetc(fp)) != EOF) {
        if (escape) {
          buffer[j++] = ch;
          escape = 0;
          continue;
        }

        if (ch == '\\') {
          buffer[j++] = ch;
          escape = 1;
          continue;
        }

        if (ch == '"') {
          buffer[j++] = '"';
          valid = 1;
          break;
        }

        if (ch == '\n') {
          line++;
          // Unterminated string - report error
          break;
        }

        // FIX: Replace spaces with underscores for token file compatibility
        if (ch == ' ') {
          buffer[j++] = '_';
        } else {
          buffer[j++] = ch;
        }
      }

      buffer[j] = '\0';

      if (valid) {
        printToken(line, buffer, "STRING_LITERAL", "STRING");
        token_count++;
      } else {
        printLexicalError(line, buffer, "Unterminated string literal");
        error_count++;
      }
    }

    // OPERATORS (MAXIMAL MUNCH)
    else if (ch == '=') {
      char next = fgetc(fp);
      if (next == '=') {
        printToken(line, "==", "OPERATOR_EQUAL", "OPERATOR");
        token_count++;
      } else {
        printToken(line, "=", "OPERATOR_ASSIGN", "OPERATOR");
        token_count++;
        if (next != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    else if (ch == '>') {
      char next = fgetc(fp);
      if (next == '=') {
        printToken(line, ">=", "OPERATOR_GREATER_EQUAL", "OPERATOR");
        token_count++;
      } else {
        printToken(line, ">", "OPERATOR_GREATER_THAN", "OPERATOR");
        token_count++;
        if (next != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    else if (ch == '<') {
      char next = fgetc(fp);
      if (next == '=') {
        printToken(line, "<=", "OPERATOR_LESS_EQUAL", "OPERATOR");
        token_count++;
      } else {
        printToken(line, "<", "OPERATOR_LESS_THAN", "OPERATOR");
        token_count++;
        if (next != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    else if (ch == '!') {
      char next = fgetc(fp);
      if (next == '=') {
        printToken(line, "!=", "OPERATOR_NOT_EQUAL", "OPERATOR");
        token_count++;
      } else {
        printLexicalError(line, "!",
                          "Invalid operator '!' (did you mean '!=')?");
        error_count++;
      }
    }

    else if (ch == '+') {
      char next = fgetc(fp);
      if (next == '+') {
        printLexicalError(
            line, "++", "Operator '++' not supported in this language subset");
        error_count++;
      } else {
        printToken(line, "+", "OPERATOR_PLUS", "OPERATOR");
        token_count++;
        if (next != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    else if (ch == '-') {
      char next = fgetc(fp);
      if (next == '-') {
        printLexicalError(
            line, "--", "Operator '--' not supported in this language subset");
        error_count++;
      } else {
        printToken(line, "-", "OPERATOR_MINUS", "OPERATOR");
        token_count++;
        if (next != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    else if (ch == '*') {
      printToken(line, "*", "OPERATOR_MULTIPLICATION", "OPERATOR");
      token_count++;
    }

    else if (ch == '%') {
      printToken(line, "%", "OPERATOR_MODULO", "OPERATOR");
      token_count++;
    }

    else if (ch == '&') {
      char next = fgetc(fp);
      if (next == '&') {
        printLexicalError(
            line, "&&", "Operator '&&' not supported in this language subset");
        error_count++;
      } else {
        printLexicalError(line, "&",
                          "Invalid operator '&' (bitwise AND not supported)");
        error_count++;
        if (next != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    else if (ch == '|') {
      char next = fgetc(fp);
      if (next == '|') {
        printLexicalError(
            line, "||", "Operator '||' not supported in this language subset");
        error_count++;
      } else {
        printLexicalError(line, "|",
                          "Invalid operator '|' (bitwise OR not supported)");
        error_count++;
        if (next != EOF) {
          fseek(fp, -1, SEEK_CUR);
        }
      }
    }

    // PUNCTUATORS
    else if (ch == '(') {
      printToken(line, "(", "LEFT_PARENTHESIS", "PUNCTUATOR");
      token_count++;
    }

    else if (ch == ')') {
      printToken(line, ")", "RIGHT_PARENTHESIS", "PUNCTUATOR");
      token_count++;
    }

    else if (ch == '{') {
      printToken(line, "{", "LEFT_BRACKETS", "PUNCTUATOR");
      token_count++;
    }

    else if (ch == '}') {
      printToken(line, "}", "RIGHT_BRACKETS", "PUNCTUATOR");
      token_count++;
    }

    else if (ch == ';') {
      printToken(line, ";", "SEMICOLON", "PUNCTUATOR");
      token_count++;
    }

    else if (ch == ',') {
      printToken(line, ",", "COMMA", "PUNCTUATOR");
      token_count++;
    }

    // CATCH-ALL ERROR (VERY IMPORTANT)
    else {
      char err[2] = {ch, '\0'};
      printLexicalError(line, err, "Unknown/unrecognized symbol");
      error_count++;
    }
  }

  fclose(fp);

  // Print summary to stderr
  fprintf(stderr, "\n[SCANNER SUMMARY]\n");
  fprintf(stderr, "  Total Tokens: %d\n", token_count);
  fprintf(stderr, "  Lexical Errors: %d\n", error_count);
  if (error_count > 0) {
    fprintf(stderr, "  Status: COMPLETED WITH ERRORS\n");
    fprintf(stderr, "  NOTE: Fix lexical errors before running parser.\n");
  } else {
    fprintf(stderr, "  Status: SUCCESS\n");
  }
  fprintf(stderr, "-------------------\n");

  return error_count > 0 ? 1 : 0;
}
