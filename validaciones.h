/*
 * validaciones.h - Ingreso seguro de datos y mensajes de la interfaz de consola
 */
#ifndef VALIDACIONES_H
#define VALIDACIONES_H

#define TAM_LINEA 128

int    leerEntero(const char *mensaje, int minimo, int maximo);
double leerReal(const char *mensaje, double minimo, double maximo);
char   leerOpcion(const char *mensaje, const char *validas);
int    leerSiNo(const char *mensaje);
void   pausar(void);
int    erroresDeEntrada(void);

void mostrarTitulo(const char *titulo);
void mostrarSeparador(void);
void mostrarError(const char *mensaje);
void mostrarAviso(const char *mensaje);
void mostrarOk(const char *mensaje);

#endif /* VALIDACIONES_H */
