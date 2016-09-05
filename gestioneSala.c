#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "cinema.h"
/*
 * Metodo di supporto all'inizializzazione dello stadio: alloca lo spazio nello heap
 * per le file del settore i-esimo e per i posti di tali file.
 * Inizializza file e posti del settore i-esimo.
 */
 void deb(int d, char* s){
	 if(d) printf("<DEBUG> - %s\n", s);
 }
void creaFilePosti(int i, int nfile, int npostifila){
	//TODO controllare se esiste già una sala (su file)
	int j,k;
	sala.nFile = nfile;
	sala.file = malloc(sizeof(struct fila)*nfile);
	for (j = 0; j < nfile; j++){
		sala.file[j].nPosti = npostifila;
		sala.file[j].posti = malloc(sizeof(struct posto)*npostifila);
		for (k = 0; k < npostifila; k++){
			sala.file[j].posti[k].codice = k;
			sala.file[j].posti[k].codice_fila = j;
			sala.file[j].posti[k].is_prenotato = 0;
		}
		sala.file[j].codice = j;
		sala.file[j].nPostiLiberi = npostifila;
		pthread_mutex_init(&sala.file[j].mx_fila,NULL);
	}
}
// inizializza sala cinema TODO : controllare per sala già esistente
void inizializzaSala(){
	int i;
	int nfile=0;
	int npostifila=0;
	int costoIntero=0;
	pthread_mutexattr_t attr;

	//inizializzo sala
		nfile = 35;
		npostifila = 30;
		costoIntero = 20;
		creaFilePosti(i, nfile, npostifila);

		sala.postiDisponibili = nfile * npostifila;
		//strcpy(settori[i].nome,nomiSettori[i]);
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
		pthread_mutex_init(&sala.mx_aggiornamentoSala,&attr);
		pthread_mutexattr_destroy(&attr);

		printf("<SERVER-INFO> cinema Inizializzato\n");
}

//stampa sala
void stampaSala(struct sala_cinema s){
	int j,k;
	for (j = 0; j < s.nFile; j++){
		for (k = 0; k < s.file[j].nPosti; k++){
			printf("%d\n", s.file[j].posti[k].is_prenotato);
		}
		printf("\n");
	}
}

//stampa in una stringa le informazioni sul settore
/*void stampaDescrizioneSettore(char* stringa, struct sala_cinema* s){
	pthread_mutex_lock(&(s->mx_aggiornamentoSala));
	sprintf(stringa,"Codice: %d, %s, posti liberi: %d, prezzo: %d, posti fila: %d\n",s->codice +1,s->nome,s->postiDisponibili, s->costoIntero, s->file[0].nPosti);
	pthread_mutex_unlock(&(s->mx_aggiornamentoSala));
}*/

/*
 * Algoritmo che prenota i posti nello stadio: appena trova un posto libero
 * in una fila del settore specificato dall'utente, verifica che ci siano n
 * posti adiacenti accanto (dove n è specificato dal client); se ci sono
 * prenota tutto, altrimenti va avanti. Dopo aver scorso tutte le file,
 * se non trova n posti adiacenti restituisce 0 (esito negativo), altrimenti
 * 1. Ritorna -1 se nel settore non ci sono sufficienti posti.
 */
/*int prenotaPosti(int prenotazione[2], int client){
	int i,j,y;	//per ciclare su settore, fila e posto
	int adiacentiLiberi = 0;	//contatore
	int prenotati = 0;	//risultato
	//estraggo i dati forniti dal client
	//int nSettore = (prenotazione[0]);
	int postiAdiacentiRichiesti = prenotazione[1];

	//controllo se i posti sono sufficienti: non accadrà mai, perchè il controllo è
	//già effettuato in server.c
	pthread_mutex_lock(&sala.mx_aggiornamentoSala);
	if (sala.postiDisponibili<postiAdiacentiRichiesti) {
		printf("<SERVER-INFO> Il client %i ha richiesto più posti di quelli disponibili nel settore %s\n",client, sala.nome);
		return -1;
	}
	pthread_mutex_unlock(&sala.mx_aggiornamentoSala);

	//per tutte le file della sala
	for (i = 0; i < sala.nFile; i++) {
		//acquisisco il lock sulla fila
		pthread_mutex_lock(&sala.file[i].mx_fila);
		//se la fila ha più posti di quelli richiesti e se ci sono posti a sufficienza
		if (sala.file[i].nPostiLiberi >= postiAdiacentiRichiesti){
			//scandisco i posti della fila
			for (j = 0; j < sala.file[i].nPosti; j++){
				//se il posto j non è stato ancora prenotato
				if (!sala.file[i].posti[j].is_prenotato){
					for (y = j; y < (j+postiAdiacentiRichiesti); y++){
						//conto quanti posti liberi adiacenti ci sono
						if (!sala.file[i].posti[y].is_prenotato){
							adiacentiLiberi++;
						}
					}
					//se bastano i posti liberi adiacenti
					if (adiacentiLiberi == postiAdiacentiRichiesti){
						for (y = j; y < (j+postiAdiacentiRichiesti); y++) {
							//prenoto il posto
							sala.file[i].posti[y].is_prenotato == 1;
							//la fila ha un posto disponibile in meno
							sala.file[i].nPostiLiberi--;
						}
						prenotati = 1;
						//aggiorno il settore
						pthread_mutex_lock(&sala.mx_aggiornamentoSala);
								sala.postiDisponibili = sala.postiDisponibili-postiAdiacentiRichiesti;
						pthread_mutex_unlock(&sala.mx_aggiornamentoSala);
						printf("<SERVER-UPDATE> Posti disponibili nel settore %d: %d\n",nSettore,sala.postiDisponibili);
					}
					//se abbiamo trovato i posti, stop
					if (prenotati)
						break;
					//altrimenti resetto
					adiacentiLiberi = 0;
				}
			}
		}
		pthread_mutex_unlock(&sala.file[i].mx_fila);

		if (prenotati)
			break;
	}
	return prenotati;
}*/
