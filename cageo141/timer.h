#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#define TIMER_STOPPED 0
#define TIMER_RUNNING 1
#define TIMER_PAUSED 2

/** @brief Cronmetro para medir tiempos de ejecucin de algoritmos */

class Timer{
  /** Tiempo inicial */
  timeval tini;
  /* Acumulador de segundos */
  unsigned sec;
  /* Acumulador de microsegundos */
  long usec;

  /* Estado del cronmetro: parado, en funcionamiento, en pausa */
  int state;
  /* Actualizar los acumuladores de segundos y microsegundos */
  void updateTime ();
  
public:
    /** Inicia el cronmetro */
    Timer() { sec = 0; usec = 0; state = TIMER_STOPPED; }
    /** Comenzar medicin de tiempo */
    void start ();
    /** Realizar una pausa en la medicin */
    void pause ();
    /** Continuar medicin de tiempo tras una pausa */
    void resume ();
    /** Terminar la medicin de tiempo */
    void stop ();
    
    /** Obtener el tiempo en segundos. El cronmetro debe estar parado */
    float timeSecs () { return (float) sec + (float) usec / 1000000.0f; }
    /** Obtener el tiempo en milisegundos. El cronmetro de estar parado */
    float timeMSecs () { return (float) sec * 1000.0f + (float) usec / 1000.0f; }
    /** Obtener el tiempo en microsegundos. El cronmetro debe estar parado */
    float timeUSecs () { return (float) sec * 1000000.0f + (float) usec; }
};

#endif
