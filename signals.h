#ifndef __SIGNALS_H__
#define __SIGNALS_H__

// Initialise la gestion des signaux utilisées
// par le shell (assigne les handlers et
// prépare les bloquages)
void signals_init ();

// Bloque les signaux importants pour le shell
// s'ils ne sont pas déjà bloqués
void signals_lock (char* desc);

// Débloque les signaux importants pour le shell
// s'ils ont été bloqués au préalable par un
// appel à `signals_lock`
void signals_unlock (char* desc);

#endif