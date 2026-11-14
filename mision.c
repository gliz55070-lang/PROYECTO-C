/*
 * mision.c - Inciso d (juego de planificacion de combustible por niveles)
 * y reporte final de desempenio (por pantalla y en archivo de texto).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "artemis.h"
#include "validaciones.h"

/*
 * Un nivel del juego: el jugador decide cuantos kg de combustible cargar para
 * lograr 'dv_necesario' empujando una nave de 'masa_final' kg (sin ese combustible).
 * Devuelve los puntos obtenidos (10 / 7 / 4 segun el intento, la mitad si sobra mucho)
 * y deja en *carga el combustible con el que se sigue jugando.
 */
static int jugarCarga(const char *nombre, double masa_final, double dv_necesario,
                      double ce, double *carga)
{
    int intento, puntos = 0;
    double necesario = combustibleNecesario(masa_final, dv_necesario, ce);
    double dv;

    printf("\n  >> %s\n", nombre);
    printf("     Masa a empujar (sin este combustible): %.0f kg\n", masa_final);
    printf("     Delta-v necesario: %.1f m/s     ce = Isp*g0 = %.1f m/s\n", dv_necesario, ce);
    printf("     Tsiolkovsky: dv = ce * ln(m0/mf)\n");

    for (intento = 1; intento <= MAX_INTENTOS; intento++) {
        printf("     Intento %d de %d. ", intento, MAX_INTENTOS);
        *carga = leerReal("Combustible a cargar [kg] (0 a 10000000): ", 0.0, 1e7);
        dv = deltaVObtenido(masa_final, *carga, ce);
        printf("     Encendido... dv obtenido = %.1f m/s (necesario %.1f m/s)\n", dv, dv_necesario);

        if (dv < dv_necesario * 0.999) {
            printf("     [X] No alcanza: faltan %.1f m/s.\n", dv_necesario - dv);
            if (intento < MAX_INTENTOS)
                printf("     Pista: m0/mf tiene que ser e^(dv/ce) = %.3f\n", exp(dv_necesario / ce));
            continue;
        }
        puntos = (intento == 1) ? 10 : (intento == 2 ? 7 : 4);
        if (*carga > necesario * (1.0 + MARGEN_SOBRA)) {
            puntos /= 2;
            printf("     [!] Alcanza, pero cargaste %.0f kg de mas (%.0f %%). Ese peso extra\n",
                   *carga - necesario, (*carga / necesario - 1.0) * 100.0);
            printf("         tambien hay que acelerarlo y rinde poco: dv crece con el LOGARITMO de la masa.\n");
        } else {
            printf("     [OK] Justo! Necesitabas %.0f kg.\n", necesario);
        }
        printf("     (+%d puntos)\n", puntos);
        return puntos;
    }
    printf("     Se terminaron los intentos. Lo necesario era %.0f kg; se sigue con ese valor.\n", necesario);
    *carga = necesario;
    return 0;
}

void incisoD(const Parametros *p, Reporte *rep)
{
    AnalisisCombustible c;
    double carga_ida1, carga_vuelta2, carga_vuelta3, carga_ida3;
    double dv_con_ida1;
    int pts1, pts2, pts3a, pts3b, ptsPred = 0;
    char resp;
    char evento[TAM_EVENTO];

    analizarCombustible(p, &c);

    mostrarTitulo("INCISO d) POR QUE CUESTA MAS IR QUE VOLVER? (JUEGO DE COMBUSTIBLE)");
    printf("  Sos el planificador de la mision. Orion pesa %.0f kg sin combustible.\n", p->masa);
    printf("  Hay dos encendidos: en LEO para IR y en orbita lunar (100 km) para VOLVER.\n");
    printf("  Si cargas de menos no llegas; si cargas de mas, ese peso tambien hay que moverlo.\n");
    printf("  Simplificacion: se usa el mismo motor (Isp = %.0f s) en los dos encendidos,\n", p->isp);
    printf("  para que la diferencia dependa solo de la fisica. En la mision real el\n");
    printf("  encendido TLI lo hace la etapa ICPS (motor RL10, Isp cercano a 460 s).\n");

    /* Nivel 1: la ida */
    printf("\n  NIVEL 1 - LA IDA: de la orbita circular (%.1f m/s) a v_TLI (%.1f m/s).\n",
           c.v_circ_leo, p->vtli);
    pts1 = jugarCarga("Encendido de ida", p->masa, c.dv_ida, c.ce, &carga_ida1);

    /* Nivel 2: prediccion + la vuelta */
    printf("\n  NIVEL 2 - LA VUELTA: escapar de la orbita lunar baja.\n");
    printf("  Prediccion: cuanto combustible hara falta para volver?\n");
    printf("    a) Lo mismo que para ir: la distancia es la misma\n");
    printf("    b) Un poco menos (unos 3/4 de la ida)\n");
    printf("    c) Mucho menos: el pozo gravitatorio de la Luna es mucho menos profundo\n");
    printf("    d) Mas: hay que frenar para no estrellarse contra la Tierra\n");
    resp = leerOpcion("  Tu prediccion (a/b/c/d): ", "abcd");
    if (resp == 'c') {
        mostrarOk("Bien predicho. (+5 puntos)");
        ptsPred = 5;
    } else {
        mostrarAviso("No. En el espacio no hay rozamiento: lo que cuesta es salir de un pozo gravitatorio,");
        printf("          y al volver el frenado lo hace la atmosfera (escudo termico), no el motor.\n");
    }
    pts2 = jugarCarga("Encendido de vuelta", p->masa, c.dv_vuelta, c.ce, &carga_vuelta2);

    /* Nivel 3: mision completa */
    printf("\n  NIVEL 3 - MISION COMPLETA. La trampa: el combustible de la vuelta NO se carga\n");
    printf("  en la Luna. Sale de la Tierra dentro de la nave, y el encendido de ida tambien\n");
    printf("  tiene que acelerarlo.\n");
    pts3a = jugarCarga("Tanque de vuelta", p->masa, c.dv_vuelta, c.ce, &carga_vuelta3);

    dv_con_ida1 = deltaVObtenido(p->masa + carga_vuelta3, carga_ida1, c.ce);
    printf("\n  En el nivel 1 cargaste %.0f kg para la ida. Con el tanque de vuelta a bordo,\n", carga_ida1);
    printf("  esa carga daria solo %.1f m/s (hacen falta %.1f m/s).\n", dv_con_ida1, c.dv_ida);
    pts3b = jugarCarga("Tanque de ida (empujando nave + tanque de vuelta)",
                       p->masa + carga_vuelta3, c.dv_ida, c.ce, &carga_ida3);
    pts3a = (pts3a + pts3b) / 2;

    /* Justificacion energetica */
    mostrarTitulo("JUSTIFICACION ENERGETICA (respuesta al inciso d)");
    printf("  Profundidad del pozo desde la orbita baja (G*M/r, por kg):\n");
    printf("    Tierra (desde LEO):  %.2f MJ/kg\n", c.pozo_tierra / 1e6);
    printf("    Luna (desde 100 km): %.2f MJ/kg   -> el terrestre es %.1f veces mas profundo\n",
           c.pozo_luna / 1e6, c.pozo_tierra / c.pozo_luna);
    printf("  Energia que entrega cada encendido (por kg de nave):\n");
    printf("    Ida:    (v_TLI^2 - v_circ^2)/2 = %.2f MJ/kg   (dv = %.1f m/s)\n", c.e_ida / 1e6, c.dv_ida);
    printf("    Vuelta: (v_esc^2 - v_circ^2)/2 = %.2f MJ/kg   (dv = %.1f m/s)\n", c.e_vuelta / 1e6, c.dv_vuelta);
    printf("  Combustible (Tsiolkovsky, m_comb = m_f*(e^(dv/ce) - 1)):\n");
    printf("    Ida sola:  %.0f kg    Vuelta: %.0f kg    -> %.1f veces mas para ir\n",
           c.comb_ida_sola, c.comb_vuelta, c.comb_ida_sola / c.comb_vuelta);
    printf("    Ida llevando el combustible de vuelta: %.0f kg\n", c.comb_ida_total);
    printf("    Total de la mision: %.0f kg (%.2f veces la masa de la nave)\n",
           c.comb_ida_total + c.comb_vuelta, (c.comb_ida_total + c.comb_vuelta) / p->masa);
    printf("\n  Conclusion: ir cuesta mucho mas porque hay que subir el pozo gravitatorio\n");
    printf("  terrestre, mucho mas profundo que el lunar. Ademas el combustible crece en\n");
    printf("  forma EXPONENCIAL con el dv, y en la ida se acelera tambien el combustible\n");
    printf("  de la vuelta. La distancia no es lo que importa: sin rozamiento, la nave\n");
    printf("  viaja por inercia.\n");

    rep->puntos_d = pts1 + ptsPred + pts2 + pts3a;
    rep->hecho_d = 1;
    rep->comb_ida_d = carga_ida3;
    rep->comb_vuelta_d = carga_vuelta3;
    rep->p = *p;
    snprintf(evento, sizeof evento, "Inciso d: ida %.0f kg + vuelta %.0f kg de combustible (+%d).",
             carga_ida3, carga_vuelta3, rep->puntos_d);
    registrarEvento(&rep->bitacora, evento);
    printf("\n  Puntaje del inciso d: %d de %d\n", rep->puntos_d, PTS_MAX_D);
    pausar();
}
