#include "timer.h"

void Timer::updateTime ()
{
  timeval tnow;
  gettimeofday (&tnow, 0);
  
  sec += tnow.tv_sec - tini.tv_sec;
  usec += tnow.tv_usec - tini.tv_usec;
  if (usec < 0) {
    sec --;
    usec += 1000000u;
  }
}

void Timer::start () 
{ 
  if (state == TIMER_STOPPED) {
    sec = 0; usec = 0; 
    gettimeofday (&tini, 0); 
    state = TIMER_RUNNING;
  } 
}

void Timer::resume () 
{ 
  if (state == TIMER_PAUSED) { 
    gettimeofday (&tini, 0); 
    state = TIMER_RUNNING;
  }
}
    
void Timer::pause () 
{ 
  if (state == TIMER_RUNNING) { 
    updateTime (); 
    state = TIMER_PAUSED;
  }
}

void Timer::stop ()
{
  if (state == TIMER_RUNNING || state == TIMER_PAUSED) {
    if (state == TIMER_RUNNING)
      updateTime ();
    state = TIMER_STOPPED;
  }
}
