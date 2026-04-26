int main() {
 FILE *file = fopen("input.c", "r");
 lexer(file);
 fclose(file);
 return 0;
}