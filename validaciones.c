/*
 * validaciones.c - Ingreso seguro de enteros, reales y caracteres.
 *
 * Todas las lecturas usan fgets (nunca scanf directo), asi una entrada mal
 * escrita no deja basura en el buffer ni genera bucles infinitos.
 * Se valida: linea vacia, linea demasiado larga, letras donde van numeros,
 * caracteres sobrantes, desborde numerico y rango permitido.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include "validaciones.h"

static int contador_errores = 0;

int erroresDeEntrada(void)
{
    return contador_errores;
}

void mostrarSeparador(void)
{
    printf("------------------------------------------------------------------\n");
}

void mostrarTitulo(const char *titulo)
{
    printf("\n==================================================================\n");
    printf("  %s\n", titulo);
    printf("==================================================================\n");
}

void mostrarError(const char *mensaje)
{
    contador_errores++;
    printf("  [ERROR] %s\n", mensaje);
}

void mostrarAviso(const char *mensaje)
{
    printf("  [AVISO] %s\n", mensaje);
}

void mostrarOk(const char *mensaje)
{
    printf("  [OK] %s\n", mensaje);
}

/*
 * Lee una linea completa. Devuelve 1 si se leyo bien, 0 si fue demasiado larga.
 * Si se termina la entrada (Ctrl+Z / Ctrl+D), cierra el programa en forma ordenada.
 */
static int leerLinea(char *buffer, size_t tam)
{
    size_t largo;
    int c;

    if (fgets(buffer, (int)tam, stdin) == NULL) {
        printf("\n  Fin de la entrada. Se cierra el simulador.\n");
        exit(EXIT_SUCCESS);
    }
    largo = strlen(buffer);
    if (largo > 0 && buffer[largo - 1] == '\n') {
        buffer[largo - 1] = '\0';
        if (largo > 1 && buffer[largo - 2] == '\r') buffer[largo - 2] = '\0'; /* Windows */
        return 1;
    }
    /* No entro la linea completa: se descarta el resto */
    while ((c = getchar()) != '\n' && c != EOF) { }
    return 0;
}

/* Quita espacios al principio y al final (modifica la cadena) */
static char *recortar(char *s)
{
    char *fin;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    fin = s + strlen(s) - 1;
    while (fin > s && isspace((unsigned char)*fin)) {
        *fin = '\0';
        fin--;
    }
    return s;
}

/* Elimina todos los caracteres c de la cadena */
static void quitarCaracter(char *s, char c)
{
    char *dst = s;
    for (; *s != '\0'; s++)
        if (*s != c) *dst++ = *s;
    *dst = '\0';
}

/* 1 si la cadena tiene la forma 1.234 / 12.345.678 (puntos de miles) */
static int tienePuntosDeMiles(const char *s)
{
    int digitos = 0, grupos = 0;
    if (*s == '-' || *s == '+') s++;
    while (isdigit((unsigned char)*s)) { digitos++; s++; }
    if (digitos < 1 || digitos > 3) return 0;
    while (*s == '.') {
        int k;
        s++;
        for (k = 0; k < 3; k++) {
            if (!isdigit((unsigned char)*s)) return 0;
            s++;
        }
        grupos++;
    }
    return (*s == '\0' && grupos > 0);
}

/*
 * Acepta formato argentino e internacional:
 *   10900   10.900   10900,5   10.900,5   10900.5   1,5e3
 */
static void normalizarNumero(char *s)
{
    if (strchr(s, ',') != NULL) {
        quitarCaracter(s, '.');                 /* los puntos eran de miles */
        *strchr(s, ',') = '.';                  /* la coma es el decimal */
    } else if (tienePuntosDeMiles(s)) {
        quitarCaracter(s, '.');
    }
}

int leerEntero(const char *mensaje, int minimo, int maximo)
{
    char linea[TAM_LINEA];
    char *s, *fin;
    long valor;

    while (1) {
        printf("%s", mensaje);
        if (!leerLinea(linea, sizeof linea)) {
            mostrarError("La entrada es demasiado larga.");
            continue;
        }
        s = recortar(linea);
        if (*s == '\0') {
            mostrarError("No escribiste nada. Ingresa un numero entero.");
            continue;
        }
        errno = 0;
        valor = strtol(s, &fin, 10);
        if (fin == s) {
            mostrarError("Eso no es un numero entero (no se aceptan letras).");
            continue;
        }
        if (*fin != '\0') {
            mostrarError("Sobran caracteres despues del numero (no se aceptan decimales ni letras).");
            continue;
        }
        if (errno == ERANGE || valor < minimo || valor > maximo) {
            printf("  [ERROR] Fuera de rango: debe estar entre %d y %d.\n", minimo, maximo);
            contador_errores++;
            continue;
        }
        return (int)valor;
    }
}

double leerReal(const char *mensaje, double minimo, double maximo)
{
    char linea[TAM_LINEA];
    char *s, *fin;
    double valor;

    while (1) {
        printf("%s", mensaje);
        if (!leerLinea(linea, sizeof linea)) {
            mostrarError("La entrada es demasiado larga.");
            continue;
        }
        s = recortar(linea);
        if (*s == '\0') {
            mostrarError("No escribiste nada. Ingresa un numero.");
            continue;
        }
        normalizarNumero(s);
        errno = 0;
        valor = strtod(s, &fin);
        if (fin == s) {
            mostrarError("Eso no es un numero (no se aceptan letras).");
            continue;
        }
        if (*fin != '\0') {
            mostrarError("Sobran caracteres despues del numero. Ejemplos validos: 10900  10.900  10900,5");
            continue;
        }
        if (errno == ERANGE || !isfinite(valor)) {
            mostrarError("El numero es demasiado grande o demasiado chico.");
            continue;
        }
        if (valor < minimo || valor > maximo) {
            printf("  [ERROR] Fuera de rango: debe estar entre %.10g y %.10g.\n", minimo, maximo);
            contador_errores++;
            continue;
        }
        return valor;
    }
}

/* Lee UN caracter que este en la lista 'validas' (sin distinguir mayusculas) */
char leerOpcion(const char *mensaje, const char *validas)
{
    char linea[TAM_LINEA];
    char *s;
    char c;

    while (1) {
        printf("%s", mensaje);
        if (!leerLinea(linea, sizeof linea)) {
            mostrarError("La entrada es demasiado larga.");
            continue;
        }
        s = recortar(linea);
        if (strlen(s) != 1) {
            printf("  [ERROR] Escribi una sola letra de estas: %s\n", validas);
            contador_errores++;
            continue;
        }
        c = (char)tolower((unsigned char)s[0]);
        if (strchr(validas, c) == NULL) {
            printf("  [ERROR] Opcion invalida. Validas: %s\n", validas);
            contador_errores++;
            continue;
        }
        return c;
    }
}

int leerSiNo(const char *mensaje)
{
    return leerOpcion(mensaje, "sn") == 's';
}

void pausar(void)
{
    char linea[TAM_LINEA];
    printf("\n  Presiona ENTER para continuar...");
    leerLinea(linea, sizeof linea);
}
