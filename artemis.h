/*
 * artemis.h - Modulo de cabecera del simulador
 * Situacion-Problema 3: Inyeccion Translunar (TLI) e inercia en trayectoria libre
 *
 * Contiene: constantes (#define), tipos de datos (struct / typedef)
 * y prototipos de las funciones publicas de la simulacion.
 */
#ifndef ARTEMIS_H
#define ARTEMIS_H

/* ---------------- Constantes fisicas (SI) ---------------- */
#define G_UNIV      6.674e-11     /* constante de gravitacion [N m^2/kg^2] (valor de la catedra) */
#define M_TIERRA    5.972e24      /* masa de la Tierra [kg] (valor de la catedra) */
#define R_TIERRA    6371000.0     /* radio medio terrestre [m] (valor de la catedra) */
#define M_LUNA      7.342e22      /* masa de la Luna [kg] */
#define R_LUNA      1737400.0     /* radio lunar [m] */
#define G0          9.80665       /* gravedad estandar, para el impulso especifico [m/s^2] */
#define H_LLO       100000.0      /* altura de la orbita lunar baja supuesta [m] */
#define J_TON_TNT   4.184e9       /* 1 tonelada de TNT [J] */
#define J_KWH       3.6e6         /* 1 kWh [J] */

/* ---------------- Datos de la consigna (valores por defecto) ---------------- */
#define V_TLI_DEF   10900.0       /* [m/s] */
#define MASA_DEF    26000.0       /* [kg]  */
#define H_LEO_DEF   192000.0      /* [m]   */
#define DIST_DEF    384000000.0   /* [m]   */
#define ISP_DEF     316.0         /* [s] motor principal de Orion (OMS-E) */

/* ---------------- Rangos validos para las entradas ---------------- */
#define V_MIN       8000.0
#define V_MAX       13000.0
#define MASA_MIN    1000.0
#define MASA_MAX    100000.0
#define H_MIN       100000.0
#define H_MAX       2000000.0
#define DIST_MIN    350000000.0
#define DIST_MAX    410000000.0
#define ISP_MIN     200.0
#define ISP_MAX     500.0

/* ---------------- Parametros de simulacion y juego ---------------- */
#define DT_SIM          60.0      /* paso de integracion RK4 [s] */
#define T_MAX_SIM       (20.0 * 86400.0) /* tiempo maximo simulado: 20 dias */
#define MUESTREO_SIM    (6.0 * 3600.0)   /* se guarda una muestra cada 6 h */
#define CAPACIDAD_INICIAL 8       /* tamanio inicial del vector dinamico de muestras */
#define MAX_BARRIDO     64
#define TAM_EVENTO      160       /* largo maximo del texto de un evento de la bitacora */
#define MAX_INTENTOS    3
#define TOLERANCIA      0.01      /* 1 % para dar por correcta una respuesta */
#define MARGEN_SOBRA    0.10      /* hasta 10 % de combustible de mas se acepta como "justo" */

/* Puntajes maximos por inciso */
#define PTS_MAX_A   5
#define PTS_MAX_B   10
#define PTS_MAX_C   5
#define PTS_MAX_D   35
#define PTS_MAX_TOTAL (PTS_MAX_A + PTS_MAX_B + PTS_MAX_C + PTS_MAX_D)

/* ---------------- Tipos de datos ---------------- */

/* Datos de entrada de la mision */
typedef struct {
    double vtli;   /* velocidad tras la TLI [m/s] */
    double masa;   /* masa de Orion [kg] */
    double hleo;   /* altura de la orbita baja [m] */
    double dist;   /* distancia Tierra-Luna (centro a centro) [m] */
    double isp;    /* impulso especifico del motor [s] */
} Parametros;

/* Estado dinamico de la nave sobre la recta Tierra-Luna */
typedef struct {
    double t;      /* tiempo desde el apagado [s] */
    double r;      /* distancia al centro de la Tierra [m] */
    double v;      /* velocidad radial [m/s] (+ hacia la Luna) */
} EstadoNave;

/* Una fila de la tabla del inciso a */
typedef struct {
    double t;       /* [s] */
    double r_mru;   /* posicion si no hubiera gravedad (1.a ley pura) [m] */
    double r_grav;  /* posicion real con gravedad [m] */
    double v_grav;  /* velocidad real [m/s] */
    double a_grav;  /* aceleracion neta [m/s^2] */
} Muestra;

/* Como termina la trayectoria simulada */
typedef enum {
    DESENLACE_LLEGA_LUNA = 0,
    DESENLACE_VUELVE_TIERRA = 1,
    DESENLACE_TIEMPO_AGOTADO = 2
} Desenlace;

typedef struct {
    int n_muestras;
    Desenlace desenlace;
    double t_final;   /* [s] */
    double r_max;     /* maxima distancia alcanzada [m] */
    double t_r_max;   /* instante de la maxima distancia [s] */
    double v_final;   /* [m/s] */
} ResultadoSimulacion;

/* Resultados del analisis energetico (inciso c) */
typedef struct {
    double r0;            /* radio inicial = R_T + h [m] */
    double xeq;           /* punto neutro gravitatorio [m] */
    double u0, ueq;       /* energia potencial por kg en r0 y en xeq [J/kg] */
    double ec_esp;        /* energia cinetica por kg = v^2/2 [J/kg] */
    double em_inicial;    /* energia mecanica por kg = ec + u0 [J/kg] */
    double delta_u;       /* U(xeq) - U(r0): "altura" del pozo a subir [J/kg] */
    double w_adicional;   /* U(xeq) - Em: trabajo extra que falta (>0) o sobra (<0) [J/kg] */
    double v_min;         /* velocidad minima para llegar al punto neutro [m/s] */
    double v_escape;      /* velocidad de escape terrestre en r0 [m/s] */
    double v_circular;    /* velocidad orbital circular en r0 [m/s] */
    double apogeo_tierra; /* apogeo si solo actuara la Tierra [m] (-1 si escapa) */
    int llega;            /* 1 si cruza el punto neutro */
} AnalisisEnergia;

/* Resultados del analisis de combustible (inciso d) */
typedef struct {
    double v_circ_leo, v_circ_llo, v_esc_llo; /* [m/s] */
    double dv_ida, dv_vuelta;                 /* [m/s] */
    double e_ida, e_vuelta;                   /* energia por kg de cada encendido [J/kg] */
    double pozo_tierra, pozo_luna;            /* profundidad de cada pozo desde la orbita baja [J/kg] */
    double ce;                                /* velocidad de salida de gases = Isp*g0 [m/s] */
    double comb_ida_sola;                     /* [kg] */
    double comb_vuelta;                       /* [kg] */
    double comb_ida_total;                    /* ida llevando el combustible de vuelta [kg] */
} AnalisisCombustible;

/* Bitacora de la mision: lista simplemente enlazada con punteros a cabeza y cola.
   Cada nodo se reserva con malloc y se libera con liberarBitacora. */
typedef struct NodoEvento {
    int numero;                    /* orden del evento */
    char texto[TAM_EVENTO];        /* descripcion del evento */
    struct NodoEvento *sig;        /* siguiente nodo (NULL al final) */
} NodoEvento;

typedef struct {
    NodoEvento *cabeza;
    NodoEvento *cola;
    int cantidad;
} Bitacora;

/* Estado del juego y datos para el reporte final */
typedef struct {
    int hecho_a, hecho_b, hecho_c, hecho_d, hecho_barrido;
    int puntos_a, puntos_b, puntos_c, puntos_d;
    int intentos_b;
    Desenlace desenlace_a;
    double r_max_a;
    double k_b;               /* energia cinetica calculada [J] */
    double w_adicional_c;     /* [J/kg] */
    int llega_c;
    double comb_ida_d, comb_vuelta_d;  /* lo que cargo el jugador en la mision completa [kg] */
    double v_umbral_barrido;  /* primera velocidad del barrido que llega [m/s] */
    Parametros p;             /* parametros con los que se jugo por ultima vez */
    Bitacora bitacora;        /* historial de eventos (lista enlazada dinamica) */
} Reporte;

/* ---------------- Prototipos: fisica (artemis.c) ---------------- */
void   parametrosPorDefecto(Parametros *p);
double radioInicial(const Parametros *p);
double aceleracionNeta(double r, double dist);
double potencialEspecifico(double r, double dist);
double puntoNeutro(double dist);
double energiaCinetica(double masa, double v);
void   pasoRK4(EstadoNave *e, double dist, double dt);
Muestra *simularTrayectoria(const Parametros *p, ResultadoSimulacion *res);
void   analizarEnergia(const Parametros *p, AnalisisEnergia *a);
void   analizarCombustible(const Parametros *p, AnalisisCombustible *c);
double combustibleNecesario(double masa_final, double dv, double ce);
double deltaVObtenido(double masa_final, double combustible, double ce);
int    barridoVelocidades(const Parametros *p, double v_ini, double v_fin, double paso,
                          double velocidades[], double trabajos[], int estados[], int max);

/* ---------------- Prototipos: incisos y juego (incisos.c / mision.c) ---------------- */
void iniciarReporte(Reporte *r);
void reiniciarReporte(Reporte *r);
int  registrarEvento(Bitacora *b, const char *texto);
void liberarBitacora(Bitacora *b);
void menuParametros(Parametros *p);
void mostrarParametros(const Parametros *p);
void incisoA(const Parametros *p, Reporte *rep);
void incisoB(const Parametros *p, Reporte *rep);
void incisoC(const Parametros *p, Reporte *rep);
void incisoD(const Parametros *p, Reporte *rep);
void barridoInteractivo(const Parametros *p, Reporte *rep);
void mostrarReporteFinal(const Reporte *rep);
int  guardarReporte(const Reporte *rep, const char *nombre_archivo);

#endif /* ARTEMIS_H */
