/* Caso 1: Comentario multilínea sin cerrar
   Este es un error crítico para un lexer. */

float pi = 3.1415.9; // Caso 2: Número con dos puntos decimales (Error)
int @valor$ = 100;    // Caso 3: Caracteres ilegales (@, $)
char *s = "Cadena sin cerrar; // Caso 4: Falta comilla de cierre
int x=5/0;            // Caso 5: Operadores pegados sin espacios