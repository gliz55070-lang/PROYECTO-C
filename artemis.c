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
