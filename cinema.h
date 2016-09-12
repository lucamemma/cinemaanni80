#ifndef CINEMA_H_
#define CINEMA_H_

#include <pthread.h>
#define CODLEN 6
#define NONDIS "NONDIS"

//generico posto in una fila
struct posto {
	int codice;
	int codice_fila;
	int prenotato;
};

//generica fila in un settore
struct fila{
	int codice;
	int nPostiLiberi;
	int nPosti;
	struct posto* posti;
	pthread_mutex_t mx_fila;
};

//settore dello stadio
struct sala_cinema{
	int codice;
	int nFile;
	struct fila* file;
	int postiDisponibili;
	pthread_mutex_t mx_aggiornamentoSala;
	pthread_mutex_t mx_aggiornamentoFilePrenotazioni;
	//pthread_mutex_t mx_aggiornamentoFileSala;
};

struct prenotazione{
	int nPosti;
	struct posto* posti_prenotati;
};

struct sala_cinema sala; //variabile condivisa thread server

struct prelazione {
	char *id;
	struct posto* posti_p;
};
char COD_PREN[CODLEN];
struct prelazione *prelazioni; //lista prenotazioni attive
#endif /*STADIOOLIMPICO_H_*/
