/*
 * artemis.c - Modulo de logica fisica de la simulacion
 *
 * Modelo: la nave se mueve sobre la recta que une los centros de la Tierra y
 * la Luna (problema unidimensional). Se considera la atraccion de los dos
 * cuerpos y se desprecian la rotacion del sistema y el movimiento de la Luna.
 */
#include <stdlib.h>
#include <math.h>
#include "artemis.h"

void parametrosPorDefecto(Parametros *p)
{
    p->vtli = V_TLI_DEF;
    p->masa = MASA_DEF;
    p->hleo = H_LEO_DEF;
    p->dist = DIST_DEF;
    p->isp  = ISP_DEF;
}

/* Radio inicial medido desde el CENTRO de la Tierra (no desde la superficie) */
double radioInicial(const Parametros *p)
{
    return R_TIERRA + p->hleo;
}

/* Aceleracion neta: la Tierra tira hacia atras (-) y la Luna hacia adelante (+) */
double aceleracionNeta(double r, double dist)
{
    double d_luna = dist - r;
    return -(G_UNIV * M_TIERRA) / (r * r) + (G_UNIV * M_LUNA) / (d_luna * d_luna);
}

/* Energia potencial gravitatoria por unidad de masa [J/kg] */
double potencialEspecifico(double r, double dist)
{
    return -(G_UNIV * M_TIERRA) / r - (G_UNIV * M_LUNA) / (dist - r);
}

/*
 * Punto neutro gravitatorio: donde las dos atracciones son iguales.
 * G*MT/x^2 = G*ML/(D-x)^2  =>  x = D / (1 + sqrt(ML/MT))
 * (No es el punto de Lagrange L1: L1 incluye el efecto de la rotacion del
 *  sistema Tierra-Luna y queda mas cerca, a unos 326.000 km.)
 */
double puntoNeutro(double dist)
{
    return dist / (1.0 + sqrt(M_LUNA / M_TIERRA));
}

double energiaCinetica(double masa, double v)
{
    return 0.5 * masa * v * v;
}

/*
 * Un paso de Runge-Kutta de orden 4 para el sistema
 *   dr/dt = v
 *   dv/dt = a(r)
 * El estado se modifica a traves del puntero.
 */
void pasoRK4(EstadoNave *e, double dist, double dt)
{
    double k1r = e->v;
    double k1v = aceleracionNeta(e->r, dist);
    double k2r = e->v + 0.5 * dt * k1v;
    double k2v = aceleracionNeta(e->r + 0.5 * dt * k1r, dist);
    double k3r = e->v + 0.5 * dt * k2v;
    double k3v = aceleracionNeta(e->r + 0.5 * dt * k2r, dist);
    double k4r = e->v + dt * k3v;
    double k4v = aceleracionNeta(e->r + dt * k3r, dist);

    e->r += (dt / 6.0) * (k1r + 2.0 * k2r + 2.0 * k3r + k4r);
    e->v += (dt / 6.0) * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
    e->t += dt;
}

/*
 * Agrega una muestra al vector dinamico. Si no hay lugar, duplica la capacidad
 * con realloc. Devuelve 1 si pudo agregarla, 0 si no hay memoria.
 * Recibe punteros al vector, a la cantidad y a la capacidad porque puede
 * modificar los tres.
 */
static int agregarMuestra(Muestra **vec, int *n, int *capacidad,
                          const EstadoNave *e, double r_mru, double dist)
{
    if (*n == *capacidad) {
        int nueva_cap = (*capacidad) * 2;
        Muestra *nuevo = (Muestra *)realloc(*vec, nueva_cap * sizeof(Muestra));
        if (nuevo == NULL) return 0;          /* el vector viejo sigue siendo valido */
        *vec = nuevo;
        *capacidad = nueva_cap;
    }
    (*vec)[*n].t = e->t;
    (*vec)[*n].r_mru = r_mru;
    (*vec)[*n].r_grav = e->r;
    (*vec)[*n].v_grav = e->v;
    (*vec)[*n].a_grav = aceleracionNeta(e->r, dist);
    (*n)++;
    return 1;
}

/*
 * Simula la trayectoria con motores apagados hasta que:
 *  - la nave llega a la superficie lunar, o
 *  - vuelve a pasar por la altura de la orbita baja (cae de vuelta), o
 *  - se agota el tiempo maximo.
 * En paralelo calcula la posicion que tendria sin gravedad (MRU, 1.a ley).
 *
 * Devuelve un vector de muestras reservado con malloc (la cantidad queda en
 * res->n_muestras). QUIEN LLAMA DEBE LIBERARLO CON free(). Devuelve NULL si
 * no hay memoria.
 */
Muestra *simularTrayectoria(const Parametros *p, ResultadoSimulacion *res)
{
    EstadoNave nave;
    double r0 = radioInicial(p);
    double r_limite_luna = p->dist - R_LUNA;
    double proximo_muestreo = MUESTREO_SIM;
    int n = 0;
    int capacidad = CAPACIDAD_INICIAL;
    Muestra *muestras = (Muestra *)malloc(capacidad * sizeof(Muestra));

    res->n_muestras = 0;
    if (muestras == NULL) return NULL;

    nave.t = 0.0;
    nave.r = r0;
    nave.v = p->vtli;

    res->r_max = r0;
    res->t_r_max = 0.0;
    if (!agregarMuestra(&muestras, &n, &capacidad, &nave, r0, p->dist)) {
        free(muestras);
        return NULL;
    }

    while (1) {
        pasoRK4(&nave, p->dist, DT_SIM);

        if (nave.r > res->r_max) {
            res->r_max = nave.r;
            res->t_r_max = nave.t;
        }
        if (nave.r >= r_limite_luna) {
            res->desenlace = DESENLACE_LLEGA_LUNA;
            break;
        }
        if (nave.r <= r0 && nave.v < 0.0) {
            res->desenlace = DESENLACE_VUELVE_TIERRA;
            break;
        }
        if (nave.t >= T_MAX_SIM) {
            res->desenlace = DESENLACE_TIEMPO_AGOTADO;
            break;
        }
        if (nave.t >= proximo_muestreo) {
            if (!agregarMuestra(&muestras, &n, &capacidad, &nave, r0 + p->vtli * nave.t, p->dist)) {
                free(muestras);
                return NULL;
            }
            proximo_muestreo += MUESTREO_SIM;
        }
    }

    /* La ultima muestra es el estado final */
    if (nave.r > r_limite_luna) nave.r = r_limite_luna;
    if (nave.r < r0) nave.r = r0;
    if (!agregarMuestra(&muestras, &n, &capacidad, &nave, r0 + p->vtli * nave.t, p->dist)) {
        free(muestras);
        return NULL;
    }

    res->n_muestras = n;
    res->t_final = nave.t;
    res->v_final = nave.v;
    return muestras;
}
