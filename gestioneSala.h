#ifndef GESTIONESALA_H_
#define GESTIONESALA_H_

#include "cinema.h"

char** split_line(char* s);
void inizializzaSala();
void creaFilePosti(int i, int nfile, int npostifila);
void stampaSala(struct sala_cinema s);

#endif /*GESTIONESTADIO_H_*/
