/*
 * main.c - Modulo principal: menu de navegacion y flujo general.
 * Simulador "Llega Orion a la Luna?" - Situacion-Problema 3 (Artemis)
 */
#include <stdio.h>
#include "artemis.h"
#include "validaciones.h"

#define ARCHIVO_REPORTE "reporte_mision.txt"

int main(void)
{
    Parametros datos;
    Reporte reporte;
    int opcion;

    parametrosPorDefecto(&datos);
    iniciarReporte(&reporte);

    mostrarTitulo("LLEGA ORION A LA LUNA?  -  Simulador de Inyeccion Translunar (TLI)");
    printf("  Mision Artemis - Situacion-Problema 3: inercia en trayectoria libre\n");
    printf("  (1.a ley de Newton y energia). Responde los incisos a, b, c y d\n");
    printf("  con los datos de la consigna o con los que vos ingreses.\n");

    do {
        mostrarTitulo("MENU PRINCIPAL");
        printf("  1) Ver / modificar datos de la mision\n");
        printf("  2) Inciso a: 1.a ley de Newton (simulacion con y sin gravedad)\n");
        printf("  3) Inciso b: energia cinetica inicial (desafio)\n");
        printf("  4) Inciso c: conservacion de la energia y trabajo hasta el punto neutro\n");
        printf("  5) Inciso d: ida vs. vuelta (juego de combustible)\n");
        printf("  6) Barrido de velocidades\n");
        printf("  7) Ver reporte de desempenio\n");
        printf("  8) Guardar reporte en '%s'\n", ARCHIVO_REPORTE);
        printf("  9) Reiniciar puntajes\n");
        printf("  0) Salir\n");
        opcion = leerEntero("\n  Elegi una opcion (0-9): ", 0, 9);

        switch (opcion) {
        case 1: menuParametros(&datos); break;
        case 2: incisoA(&datos, &reporte); break;
        case 3: incisoB(&datos, &reporte); break;
        case 4: incisoC(&datos, &reporte); break;
        case 5: incisoD(&datos, &reporte); break;
        case 6: barridoInteractivo(&datos, &reporte); break;
        case 7: mostrarReporteFinal(&reporte); pausar(); break;
        case 8:
            if (guardarReporte(&reporte, ARCHIVO_REPORTE))
                mostrarOk("Reporte guardado.");
            else
                mostrarError("No se pudo crear el archivo del reporte.");
            break;
        case 9:
            if (leerSiNo("  Seguro que queres borrar los puntajes? (s/n): ")) {
                reiniciarReporte(&reporte);
                mostrarOk("Puntajes reiniciados.");
            }
            break;
        default: break;
        }
    } while (opcion != 0);

    mostrarReporteFinal(&reporte);
    if (leerSiNo("  Guardar el reporte antes de salir? (s/n): ")) {
        if (guardarReporte(&reporte, ARCHIVO_REPORTE))
            mostrarOk("Reporte guardado en " ARCHIVO_REPORTE ".");
        else
            mostrarError("No se pudo guardar el reporte.");
    }
    liberarBitacora(&reporte.bitacora);   /* se devuelve toda la memoria dinamica */
    printf("\n  Fin de la simulacion. Hasta la proxima mision!\n");
    return 0;
}
