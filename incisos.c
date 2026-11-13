/*
 * incisos.c - Datos de la mision e incisos a, b y c, y barrido de velocidades.
 * Cada inciso calcula con las funciones de artemis.c, muestra los resultados
 * y propone una pregunta o desafio que suma puntos al reporte final.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "artemis.h"
#include "validaciones.h"

/* Deja el reporte en cero con la bitacora vacia. Usar UNA vez al principio. */
void iniciarReporte(Reporte *r)
{
    r->hecho_a = r->hecho_b = r->hecho_c = r->hecho_d = r->hecho_barrido = 0;
    r->puntos_a = r->puntos_b = r->puntos_c = r->puntos_d = 0;
    r->intentos_b = 0;
    r->desenlace_a = DESENLACE_TIEMPO_AGOTADO;
    r->r_max_a = 0.0;
    r->k_b = 0.0;
    r->w_adicional_c = 0.0;
    r->llega_c = 0;
    r->comb_ida_d = r->comb_vuelta_d = 0.0;
    r->v_umbral_barrido = 0.0;
    parametrosPorDefecto(&r->p);
    r->bitacora.cabeza = NULL;
    r->bitacora.cola = NULL;
    r->bitacora.cantidad = 0;
}

/* Libera la memoria de la bitacora y vuelve a empezar */
void reiniciarReporte(Reporte *r)
{
    liberarBitacora(&r->bitacora);
    iniciarReporte(r);
}

/* ======================= DATOS DE LA MISION ======================= */

void mostrarParametros(const Parametros *p)
{
    printf("  1) Velocidad tras la TLI ......... %10.1f m/s\n", p->vtli);
    printf("  2) Masa de Orion ................. %10.1f kg\n", p->masa);
    printf("  3) Altura de la orbita baja ...... %10.1f m  (%.1f km)\n", p->hleo, p->hleo / 1000.0);
    printf("  4) Distancia Tierra-Luna ......... %10.0f m  (%.0f km)\n", p->dist, p->dist / 1000.0);
    printf("  5) Impulso especifico (Isp) ...... %10.1f s  (solo inciso d)\n", p->isp);
}

void menuParametros(Parametros *p)
{
    int op;
    do {
        mostrarTitulo("DATOS DE LA MISION");
        mostrarParametros(p);
        printf("  6) Restaurar los datos de la consigna\n");
        printf("  0) Volver al menu principal\n");
        op = leerEntero("\n  Que dato queres cambiar? (0-6): ", 0, 6);
        switch (op) {
        case 1: p->vtli = leerReal("  Nueva velocidad TLI [m/s] (8000 a 13000): ", V_MIN, V_MAX); break;
        case 2: p->masa = leerReal("  Nueva masa [kg] (1000 a 100000): ", MASA_MIN, MASA_MAX); break;
        case 3: p->hleo = leerReal("  Nueva altura de LEO [m] (100000 a 2000000): ", H_MIN, H_MAX); break;
        case 4: p->dist = leerReal("  Nueva distancia Tierra-Luna [m] (350000000 a 410000000): ", DIST_MIN, DIST_MAX); break;
        case 5: p->isp  = leerReal("  Nuevo Isp [s] (200 a 500): ", ISP_MIN, ISP_MAX); break;
        case 6: parametrosPorDefecto(p); mostrarOk("Se restauraron los datos de la consigna."); break;
        default: break;
        }
        if (op >= 1 && op <= 5) mostrarOk("Dato actualizado.");
    } while (op != 0);
}

/* ======================= INCISO B ======================= */

/* Compara con tolerancia relativa */
static int cerca(double x, double objetivo)
{
    return fabs(x - objetivo) <= fabs(objetivo) * TOLERANCIA;
}

/* Devuelve un diagnostico del error mas probable del usuario */
static const char *diagnosticoB(double resp_gj, const Parametros *p)
{
    double k_gj = energiaCinetica(p->masa, p->vtli) / 1e9;
    if (cerca(resp_gj, 2.0 * k_gj)) return "Te dio el doble: te olvidaste del 1/2 de K = 1/2*m*v^2.";
    if (cerca(resp_gj, 0.5 * p->masa * p->vtli / 1e9) || cerca(resp_gj, p->masa * p->vtli / 1e9))
        return "Falta elevar la velocidad al cuadrado (v^2).";
    if (cerca(resp_gj, k_gj / 1e6) || cerca(resp_gj, 2.0 * k_gj / 1e6))
        return "Te dio un millon de veces menos: usaste la velocidad en km/s. En el SI va en m/s.";
    if (cerca(resp_gj, k_gj * 1e9)) return "Ese es el resultado en joules; se pide en GJ (dividi por 10^9).";
    if (cerca(resp_gj, k_gj * 1e3) || cerca(resp_gj, k_gj / 1e3))
        return "Te corriste un factor 1000: revisa el prefijo (mega = 10^6, giga = 10^9).";
    return "No coincide. Hacelo en orden: v^2, despues por m, despues por 1/2 y al final dividi por 10^9.";
}

void incisoB(const Parametros *p, Reporte *rep)
{
    double k = energiaCinetica(p->masa, p->vtli);
    double k_gj = k / 1e9;
    double resp;
    int intento, acerto = 0;
    char evento[TAM_EVENTO];

    mostrarTitulo("INCISO b) ENERGIA CINETICA INICIAL");
    printf("  La energia cinetica es la energia que tiene un cuerpo por moverse:\n");
    printf("      K = 1/2 * m * v^2      (m en kg, v en m/s  ->  K en joules)\n");
    printf("  Datos: m = %.1f kg   v = %.1f m/s\n\n", p->masa, p->vtli);
    printf("  DESAFIO: calcula K y escribila en GIGAJOULES (1 GJ = 10^9 J).\n");
    printf("  Tenes %d intentos. Puntos: 10 / 7 / 4 segun el intento.\n\n", MAX_INTENTOS);

    for (intento = 1; intento <= MAX_INTENTOS && !acerto; intento++) {
        printf("  Intento %d de %d. ", intento, MAX_INTENTOS);
        resp = leerReal("K en GJ: ", -1e30, 1e30);
        if (cerca(resp, k_gj)) {
            acerto = 1;
            rep->puntos_b = (intento == 1) ? 10 : (intento == 2 ? 7 : 4);
            printf("  [OK] Correcto! (+%d puntos)\n", rep->puntos_b);
        } else {
            printf("  [X] %s\n", diagnosticoB(resp, p));
        }
    }
    if (!acerto) {
        rep->puntos_b = 0;
        mostrarAviso("Se terminaron los intentos. Mira la resolucion:");
    }
    rep->intentos_b = intento - 1;

    printf("\n  Resolucion paso a paso:\n");
    printf("    1) v^2     = (%.1f)^2 = %.4e m^2/s^2\n", p->vtli, p->vtli * p->vtli);
    printf("    2) m*v^2   = %.1f * %.4e = %.4e kg*m^2/s^2\n", p->masa, p->vtli * p->vtli, p->masa * p->vtli * p->vtli);
    printf("    3) K       = 1/2 * %.4e = %.5e J\n", p->masa * p->vtli * p->vtli, k);
    printf("    4) En GJ   = %.5e / 10^9 = %.2f GJ\n", k, k_gj);
    printf("\n  Para dimensionarlo:\n");
    printf("    - %.1f toneladas de TNT\n", k / J_TON_TNT);
    printf("    - %.0f kWh (unos %.0f hogares durante un anio, a 3000 kWh/anio)\n", k / J_KWH, k / J_KWH / 3000.0);
    printf("  La masa entra con exponente 1 y la velocidad al cuadrado:\n");
    printf("  el doble de masa duplica K; el doble de velocidad la cuadruplica.\n");

    rep->hecho_b = 1;
    rep->k_b = k;
    rep->p = *p;
    snprintf(evento, sizeof evento, "Inciso b: K = %.2f GJ, %s en %d intento/s (+%d).",
             k_gj, acerto ? "resuelto" : "no resuelto", rep->intentos_b, rep->puntos_b);
    registrarEvento(&rep->bitacora, evento);
    pausar();
}
