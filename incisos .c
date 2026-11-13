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

/* ======================= INCISO A ======================= */

void incisoA(const Parametros *p, Reporte *rep)
{
    Muestra *muestras;            /* vector dinamico: lo reserva simularTrayectoria */
    ResultadoSimulacion res;
    char evento[TAM_EVENTO];
    int i;
    char resp;
    double r0 = radioInicial(p);
    double a0 = G_UNIV * M_TIERRA / (r0 * r0);

    mostrarTitulo("INCISO a) 1.a LEY DE NEWTON: INERCIA CON MOTORES APAGADOS");
    printf("  Al terminar la TLI la etapa ICPS se separa y los motores se apagan.\n");
    printf("  Ya no hay EMPUJE. Por la 1.a ley (inercia), la nave seguiria en linea\n");
    printf("  recta a velocidad constante si sobre ella no actuara ninguna fuerza.\n");
    printf("  Pero la fuerza neta NO es cero: la Tierra la atrae con F = G*M*m/r^2,\n");
    printf("  en sentido contrario al movimiento, y la va frenando. Como r crece,\n");
    printf("  esa fuerza se achica: el frenado es cada vez mas suave.\n\n");

    printf("  Pregunta: con los motores apagados, que mantiene a Orion en movimiento?\n");
    printf("    a) Un empuje residual de los motores\n");
    printf("    b) Nada: sigue moviendose por inercia; solo la gravedad cambia su velocidad\n");
    printf("    c) La atraccion de la Luna, que la arrastra desde el principio\n");
    resp = leerOpcion("  Tu respuesta (a/b/c): ", "abc");
    if (resp == 'b') {
        mostrarOk("Correcto. No hace falta una fuerza para mantener el movimiento: hace falta para cambiarlo. (+5 puntos)");
        rep->puntos_a = PTS_MAX_A;
    } else {
        mostrarAviso("No. Sin fuerza neta un cuerpo conserva su velocidad (1.a ley). La gravedad es la que la cambia.");
        rep->puntos_a = 0;
    }

    printf("\n  Aceleracion gravitatoria terrestre al inicio: a = G*MT/r0^2 = %.3f m/s^2\n", a0);
    printf("  Simulando la trayectoria con Runge-Kutta 4 (paso de %.0f s)...\n\n", DT_SIM);

    muestras = simularTrayectoria(p, &res);
    if (muestras == NULL) {
        mostrarError("No hay memoria suficiente para la simulacion.");
        pausar();
        return;
    }

    printf("  %8s | %14s | %14s | %10s | %12s\n",
           "t [h]", "r sin grav[km]", "r con grav[km]", "v [m/s]", "a neta[m/s2]");
    mostrarSeparador();
    for (i = 0; i < res.n_muestras; i++) {
        /* para no llenar la pantalla se muestra una fila cada 12 h, mas la primera y la ultima */
        int hora = (int)(muestras[i].t / 3600.0 + 0.5);
        if (i != 0 && i != res.n_muestras - 1 && hora % 12 != 0) continue;
        printf("  %8.1f | %14.0f | %14.0f | %10.1f | %12.6f\n",
               muestras[i].t / 3600.0, muestras[i].r_mru / 1000.0,
               muestras[i].r_grav / 1000.0, muestras[i].v_grav, muestras[i].a_grav);
    }
    mostrarSeparador();

    printf("  Distancia maxima alcanzada: %.0f km (a las %.1f h)\n", res.r_max / 1000.0, res.t_r_max / 3600.0);
    switch (res.desenlace) {
    case DESENLACE_LLEGA_LUNA:
        mostrarOk("Orion cruza el punto neutro y llega a la Luna.");
        break;
    case DESENLACE_VUELVE_TIERRA:
        mostrarAviso("Orion se frena antes del punto neutro, se detiene y vuelve a caer hacia la Tierra.");
        break;
    default:
        mostrarAviso("Se agoto el tiempo de simulacion (20 dias).");
        break;
    }
    printf("\n  Conclusion: sin gravedad (columna 2) la distancia crece siempre igual (MRU).\n");
    printf("  Con gravedad la velocidad baja todo el tiempo, rapido cerca de la Tierra y\n");
    printf("  cada vez mas lento lejos de ella, porque la fuerza disminuye con 1/r^2.\n");

    printf("  (Se guardaron %d muestras en memoria dinamica: %lu bytes)\n",
           res.n_muestras, (unsigned long)(res.n_muestras * sizeof(Muestra)));
    free(muestras);               /* se libera el vector reservado con malloc/realloc */

    rep->hecho_a = 1;
    rep->desenlace_a = res.desenlace;
    rep->r_max_a = res.r_max;
    rep->p = *p;
    snprintf(evento, sizeof evento, "Inciso a: v = %.1f m/s, distancia max. %.0f km, %s.",
             p->vtli, res.r_max / 1000.0,
             res.desenlace == DESENLACE_LLEGA_LUNA ? "llega a la Luna" : "vuelve a caer");
    registrarEvento(&rep->bitacora, evento);
    pausar();
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

/* ======================= INCISO C ======================= */

void incisoC(const Parametros *p, Reporte *rep)
{
    AnalisisEnergia a;
    char pred;
    int i;
    const int N = 8;
    char evento[TAM_EVENTO];

    analizarEnergia(p, &a);

    mostrarTitulo("INCISO c) CONSERVACION DE LA ENERGIA: TRABAJO HASTA EL PUNTO NEUTRO");
    printf("  Con los motores apagados solo actuan fuerzas gravitatorias (conservativas),\n");
    printf("  asi que la energia mecanica por kg se conserva:  Em = v^2/2 + U(r) = cte\n");
    printf("  con U(r) = -G*MT/r - G*ML/(D - r)\n\n");

    printf("  Antes de calcular: con v = %.1f m/s, llega Orion al punto neutro? (s/n)\n", p->vtli);
    pred = leerOpcion("  Tu prediccion: ", "sn");

    printf("\n  1) Punto neutro gravitatorio (las dos atracciones son iguales):\n");
    printf("     G*MT/x^2 = G*ML/(D-x)^2  ->  x = D / (1 + raiz(ML/MT))\n");
    printf("     x_eq = %.0f m = %.1f km desde el centro de la Tierra\n", a.xeq, a.xeq / 1000.0);
    printf("     (Ojo: no es el punto de Lagrange L1, que considera la rotacion\n");
    printf("      del sistema Tierra-Luna y queda cerca de los 326.000 km.)\n");
    printf("  2) Radio inicial: r0 = RT + h = %.0f m\n", a.r0);
    printf("  3) U(r0)   = %.2f J/kg\n", a.u0);
    printf("     U(x_eq) = %.2f J/kg\n", a.ueq);
    printf("     Altura del pozo a subir: U(x_eq) - U(r0) = %.2f J/kg\n", a.delta_u);
    printf("  4) Energia cinetica por kg: v^2/2 = %.2f J/kg\n", a.ec_esp);
    printf("     Em inicial = v^2/2 + U(r0) = %.2f J/kg\n", a.em_inicial);
    printf("  5) Para llegar a x_eq (como minimo con v = 0 alli) hace falta Em >= U(x_eq):\n");
    printf("     Wext/m = U(x_eq) - Em inicial = %.2f J/kg = %.2f kJ/kg\n", a.w_adicional, a.w_adicional / 1000.0);

    if (a.llega) {
        printf("     -> Wext/m es negativo: NO hace falta trabajo extra. Sobran %.2f kJ/kg.\n", -a.w_adicional / 1000.0);
        mostrarOk("Orion llega al punto neutro y despues la Luna la atrae.");
    } else {
        printf("     -> Wext/m es positivo: hay que entregarle esa energia extra a cada kg.\n");
        printf("        Para toda la nave: Wext = %.4e J\n", a.w_adicional * p->masa);
        mostrarAviso("Con esta velocidad Orion NO llega al punto neutro: se frena y vuelve.");
    }
    printf("  6) Velocidad minima desde LEO: v_min = raiz(2*(U(x_eq)-U(r0))) = %.1f m/s\n", a.v_min);
    printf("     (Velocidad de escape terrestre en LEO: %.1f m/s)\n", a.v_escape);
    if (a.apogeo_tierra > 0)
        printf("     Si solo actuara la Tierra, el punto mas lejano seria %.0f km.\n", a.apogeo_tierra / 1000.0);

    /* Perfil del pozo de potencial */
    printf("\n  Perfil de energia (por kg) entre LEO y la Luna:\n");
    printf("  %12s | %14s | %14s | %s\n", "r [km]", "U [MJ/kg]", "K [MJ/kg]", "estado");
    mostrarSeparador();
    for (i = 0; i <= N; i++) {
        double r = a.r0 + (a.xeq - a.r0) * i / N;
        double u = potencialEspecifico(r, p->dist);
        double kk = a.em_inicial - u;
        printf("  %12.0f | %14.4f | %14.4f | %s\n", r / 1000.0, u / 1e6,
               (kk > 0 ? kk : 0.0) / 1e6, kk >= 0 ? "pasa" : "NO alcanza (K seria negativa)");
    }
    mostrarSeparador();

    if ((pred == 's') == (a.llega != 0)) {
        mostrarOk("Tu prediccion fue correcta. (+5 puntos)");
        rep->puntos_c = PTS_MAX_C;
    } else {
        mostrarAviso("Tu prediccion no fue correcta. La energia decide, no la intuicion.");
        rep->puntos_c = 0;
    }

    rep->hecho_c = 1;
    rep->w_adicional_c = a.w_adicional;
    rep->llega_c = a.llega;
    rep->p = *p;
    snprintf(evento, sizeof evento, "Inciso c: v = %.1f m/s, Wext/m = %.2f kJ/kg, %s.",
             p->vtli, a.w_adicional / 1000.0, a.llega ? "llega" : "no llega");
    registrarEvento(&rep->bitacora, evento);
    pausar();
}

/* ======================= BARRIDO ======================= */

void barridoInteractivo(const Parametros *p, Reporte *rep)
{
    double vel[MAX_BARRIDO], trab[MAX_BARRIDO];
    int est[MAX_BARRIDO];
    double v_ini, v_fin, paso;
    int n, i;
    char evento[TAM_EVENTO];
    const char *nombres[3] = { "NO LLEGA", "LLEGA", "LLEGA (supera la v. de escape terrestre)" };

    mostrarTitulo("BARRIDO DE VELOCIDADES");
    printf("  Calcula Wext/m para muchas velocidades y muestra cual llega.\n");
    v_ini = leerReal("  Velocidad inicial [m/s] (8000 a 13000, ej. 10500): ", V_MIN, V_MAX);
    v_fin = leerReal("  Velocidad final   [m/s] (mayor que la inicial, ej. 11300): ", v_ini, V_MAX);
    paso  = leerReal("  Paso [m/s] (1 a 1000, ej. 25): ", 1.0, 1000.0);
    if ((v_fin - v_ini) / paso + 1 > MAX_BARRIDO) {
        paso = (v_fin - v_ini) / (MAX_BARRIDO - 1);
        printf("  [AVISO] Demasiados valores: se ajusto el paso a %.2f m/s (maximo %d filas).\n", paso, MAX_BARRIDO);
    }

    n = barridoVelocidades(p, v_ini, v_fin, paso, vel, trab, est, MAX_BARRIDO);

    printf("\n  %10s | %14s | %s\n", "v [m/s]", "Wext/m [kJ/kg]", "resultado");
    mostrarSeparador();
    rep->v_umbral_barrido = 0.0;
    for (i = 0; i < n; i++) {
        printf("  %10.1f | %14.2f | %s\n", vel[i], trab[i] / 1000.0, nombres[est[i]]);
        if (est[i] != 0 && rep->v_umbral_barrido == 0.0) rep->v_umbral_barrido = vel[i];
    }
    mostrarSeparador();
    if (rep->v_umbral_barrido > 0.0)
        printf("  Primera velocidad del barrido que llega: %.1f m/s\n", rep->v_umbral_barrido);
    else
        printf("  Ninguna velocidad del barrido alcanza el punto neutro.\n");

    rep->hecho_barrido = 1;
    snprintf(evento, sizeof evento, "Barrido de %.0f a %.0f m/s: primera que llega %.1f m/s.",
             v_ini, v_fin, rep->v_umbral_barrido);
    registrarEvento(&rep->bitacora, evento);
    pausar();
}
