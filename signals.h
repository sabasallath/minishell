#ifndef __SIGNALS_H__
#define __SIGNALS_H__

void signals_init ();

// Securise l'accès aux fonctions de mise à jour des jobs
void signals_lock();
void signals_unlock();

#endif