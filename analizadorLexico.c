#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/**
 * 4. DEFINICIÓN DE TOKENS EXTENDIDA
 */
typedef enum {
    KEYWORD, IDENTIFIER, INTEGER, REAL, 
    OPERATOR, DELIMITER, STRING, UNKNOWN
} TokenType;

typedef struct {
    char name[100];
    int count;
} Symbol;

Symbol symbol_table[500];
int symbol_count = 0;
int line = 1, col = 0;
int stats[8] = {0};

/**
 * FUNCIONES AUXILIARES
 */
void add_to_symbol_table(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            symbol_table[i].count++;
            return;
        }
    }
    strcpy(symbol_table[symbol_count].name, name);
    symbol_table[symbol_count].count = 1;
    symbol_count++;
}

int isKeyword(char *str) {
    const char *keywords[] = {
        "int", "float", "double", "char", "if", "else", 
        "while", "for", "return", "void", "main", "include"
    };
    for (int i = 0; i < 12; i++) {
        if (strcmp(str, keywords[i]) == 0) return 1;
    }
    return 0;
}

int get_char(FILE *f) {
    int c = fgetc(f);
    if (c == '\n') { line++; col = 0; }
    else col++;
    return c;
}

void unget_char(int c, FILE *f) {
    if (c == EOF) return;
    ungetc(c, f);
    if (c == '\n') line--;
    else col--;
}

/**
 * ANALIZADOR LÉXICO PRINCIPAL
 */
void lexer(FILE *file) {
    int c;
    while ((c = get_char(file)) != EOF) {
        if (isspace(c)) continue;

        // 1. COMENTARIOS Y DIVISIÓN (Caso Especial: Cierre de comentario)
        if (c == '/') {
            int next = get_char(file);
            if (next == '/') {
                while ((c = get_char(file)) != '\n' && c != EOF);
                continue;
            } else if (next == '*') {
                int cerrado = 0;
                while ((c = get_char(file)) != EOF) {
                    if (c == '*') {
                        if ((c = get_char(file)) == '/') { cerrado = 1; break; }
                        unget_char(c, file);
                    }
                }
                if (!cerrado) {
                    printf(">>> ERROR LÉXICO: Comentario /* sin cerrar en línea %d\n", line);
                    stats[UNKNOWN]++;
                }
                continue;
            } else {
                unget_char(next, file);
                printf("[OPERATOR: /]\t(L:%d, C:%d)\n", line, col);
                stats[OPERATOR]++;
            }
        }
        
        // 2. IDENTIFICADORES
        else if (isalpha(c) || c == '_') {
            char buffer[100]; int i = 0;
            do {
                if (i < 99) buffer[i++] = c;
                c = get_char(file);
            } while (isalnum(c) || c == '_');
            buffer[i] = '\0'; unget_char(c, file);
            if (isKeyword(buffer)) {
                printf("[KEYWORD: %s]\t(L:%d, C:%d)\n", buffer, line, col);
                stats[KEYWORD]++;
            } else {
                printf("[IDENTIFIER: %s]\t(L:%d, C:%d)\n", buffer, line, col);
                add_to_symbol_table(buffer);
                stats[IDENTIFIER]++;
            }
        }

        // 3. NÚMEROS (Caso Especial: Formato real inválido)
        else if (isdigit(c)) {
            char buffer[100]; int i = 0, dots = 0;
            do {
                if (i < 99) buffer[i++] = c;
                c = get_char(file);
                if (c == '.') { dots++; if (i < 99) buffer[i++] = c; c = get_char(file); }
            } while (isdigit(c) || c == '.');
            buffer[i] = '\0'; unget_char(c, file);
            if (dots > 1) {
                printf(">>> ERROR LÉXICO: Número mal formado '%s' (L:%d, C:%d)\n", buffer, line, col);
                stats[UNKNOWN]++;
            } else if (dots == 1) {
                printf("[NUMBER_REAL: %s]\t(L:%d, C:%d)\n", buffer, line, col);
                stats[REAL]++;
            } else {
                printf("[NUMBER_INT: %s]\t(L:%d, C:%d)\n", buffer, line, col);
                stats[INTEGER]++;
            }
        }

        // 4. CADENAS (Caso Especial: Sin comilla de cierre)
        else if (c == '"') {
            char buffer[255]; int i = 0, cerrado = 0;
            while ((c = get_char(file)) != EOF && c != '\n') {
                if (c == '"') { cerrado = 1; break; }
                if (i < 254) buffer[i++] = c;
            }
            buffer[i] = '\0';
            if (cerrado) {
                printf("[STRING: \"%s\"]\t(L:%d, C:%d)\n", buffer, line, col);
                stats[STRING]++;
            } else {
                printf(">>> ERROR LÉXICO: Cadena sin cerrar en línea %d\n", line);
                stats[UNKNOWN]++;
            }
        }

        // 5. OTROS
        else if (strchr("+-*=%<>!", c)) { printf("[OPERATOR: %c]\t(L:%d, C:%d)\n", c, line, col); stats[OPERATOR]++; }
        else if (strchr(";,()[]{}", c)) { printf("[DELIMITER: %c]\t(L:%d, C:%d)\n", c, line, col); stats[DELIMITER]++; }
        else { printf(">>> ERROR LÉXICO: Carácter '%c' inválido (L:%d, C:%d)\n", c, line, col); stats[UNKNOWN]++; }
    }
}

int main() {
    FILE *file = fopen("pruebaCasosEspeciales.c", "r");
    if (!file) {
        printf("Error: Cree un archivo 'pruebaCasosEspeciales.c' en la misma carpeta.\n");
        return 1;
    }

    printf("==================================================\n");
    printf("   RESULTADO DEL ANALIZADOR LÉXICO        \n");
    printf("==================================================\n");
    
    lexer(file);
    fclose(file);

    // FUNCIONALIDAD ADICIONAL: Resumen de Estadísticas
    printf("\n==================================================\n");
    printf("           ESTADÍSTICAS DEL ANÁLISIS              \n");
    printf("==================================================\n");
    printf("Keywords: %d | Identificadores: %d | Enteros: %d\n", stats[0], stats[1], stats[2]);
    printf("Reales: %d | Operadores: %d | Delimitadores: %d\n", stats[3], stats[4], stats[5]);
    printf("Strings: %d | Errores: %d\n", stats[6], stats[7]);

    // FUNCIONALIDAD ADICIONAL: Tabla de Símbolos
    printf("\nTABLA DE SÍMBOLOS (Identificadores encontrados):\n");
    for (int i = 0; i < symbol_count; i++) {
        printf("- %-15s (frecuencia: %d)\n", symbol_table[i].name, symbol_table[i].count);
    }
    printf("==================================================\n");

    return 0;
}