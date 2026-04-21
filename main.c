#include <stdio.h>
#include <stdlib.h>

void parse(FILE *fp);

int main() {
  // First, run the scanner to generate tokenized output
  printf("Running scanner...\n");
  int result = system("./scanner > tokens.txt");

  if (result != 0) {
    printf("Error running scanner\n");
    return 1;
  }

  // Now parse the tokens
  printf("Parsing tokens...\n");
  FILE *fp = fopen("tokens.txt", "r");

  if (!fp) {
    printf("Error opening tokens.txt\n");
    return 1;
  }

  parse(fp);

  fclose(fp);
  return 0;
}
