
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  /* per usare socket */
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>        /* gestione degli errori */

#include "cinema.h"
#include "gestioneSala.h"
//invia info sala
int info(int socket) {
	int i;
	//pthread_mutex_lock(&(sala.mx_aggiornamentoSala));
	if (write(socket, &sala, sizeof(sala))< 0) {
		printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}
 	//pthread_mutex_unlock(&(sala.mx_aggiornamentoSala));
	return socket;
}

void accettaPrenotazioneEffettuata(){
	printf("ACCETTA prenotazione effettuata\n");
}

void annullaPrenotazioneEffettuata(){
	printf("ANNULLA prenotazione effettuata\n");
}
//funzione esuguita dai thread che gestiscono i client
void* threadPrincipale(void* args){

	//torna l'id del thread chiamante
	int nClient = pthread_self();

	//socket di dialogo
	int sockmsg = (int)args;
	//messaggio benvenuto OK
	char* buffer = "benvenuto nel cinema"; int n;
   n = send(sockmsg,buffer,255,0);
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }


	while(1) {

		while (1)	{
			//leggo operazione da effettuare
			printf("<SERVER-INFO> Connesso al client con thread numero %d\n",nClient);
			int operazione=0;
			if (recv(sockmsg, &operazione, sizeof(operazione),0)<0) {
				printf("<SERVER-ERR> Errore nella read dalla socket (1) %s\n", strerror(errno));
				exit(1);
			}

			switch(operazione){
				case 1:
					//invia info al client
					deb(DEB,"mando info sala");
					//info(sockmsg);
					break;
				case 2:
					deb(DEB,"prenotazione posti sala");
					accettaPrenotazioneEffettuata(); //TODO
					break;
				case 3:
					deb(DEB,"annulla prenotazione codice");
					annullaPrenotazioneEffettuata(); //TODO
					break;
			}

			/* read prenotazione posti */ /*
			if (read(sockmsg, &prenotazione, sizeof(prenotazione))<0) {
				printf("<SERVER-ERR> Errore nella read dalla socket (1) %s\n", strerror(errno));
				exit(1);
			}
			//condizione d'uscita
			if (prenotazione[0]==0 || prenotazione[1]==0)
				break;
			else
			if (prenotazione[0]>N_SETTORI || prenotazione[0]<0) {
				char mex[]="Hai specificato un settore inesistente. Devi scegliere un settore diverso\n";
				printf("<SERVER-WARN> Il client %d ha specificato un settore inesistente\n",nClient);
				if (write(sockmsg,&mex, sizeof(mex))<0) {
					printf("<SERVER-ERR> Errore nella write alla socket (0) %s\n",strerror(errno));
					exit(1);
				}
			}
			else{
				//scrivo il corretto indice di settore


				pthread_mutex_lock(&sala.mx_aggiornamentoSettore);
				if (prenotazione[1]>settori[prenotazione[0]].postiDisponibili || prenotazione[1]<0) {
					pthread_mutex_unlock(&settori[prenotazione[0]].mx_aggiornamentoSettore);
					char mex[]="*** IL SETTORE SCELTO NON HA tutti questi posti disponibili. Indicare meno posti o cambiare settore\n";
					printf("<SERVER-WARN> Il client %d ha specificato troppi posti\n",nClient);
					if (write(sockmsg,&mex, sizeof(mex))<0) {;
						printf("<SERVER-ERR> Errore nella write alla socket (1) %s\n",strerror(errno));
						exit(1);
					}
				}
				else {
					//sblocco il mutex
					pthread_mutex_unlock(&settori[prenotazione[0]].mx_aggiornamentoSettore);
					//prenoto i posti
					if(prenotaPosti(prenotazione,nClient)) {
						char mex[256];
						sprintf(mex,"*** PRENOTATI %d posti nel settore %s.\nPremere 0 o un carattere alfanumerico per uscire.\nAltrimenti effettua una nuova scelta.\n",prenotazione[1],settori[prenotazione[0]].nome);
						printf("<SERVER-INFO> Il client %d ha prenotato %d posti nel settore %s.\n",nClient,prenotazione[1],settori[prenotazione[0]].nome);
						if (write(sockmsg,&mex, sizeof(mex))<0) {
							printf("<SERVER-ERR> Errore nella write alla socket (2) %s\n",strerror(errno));
							exit(1);
						}
					}
					else {
						char mex[]="Posti non disponibili. Premere 0 o un carattere alfanumerico per uscire.\nAltrimenti effettua una nuova scelta.\n";
						printf("<SERVER-INFO> Nel settore %s non sono presenti %d posti adiacenti come richiesti dal client %d\n",settori[prenotazione[0]].nome,prenotazione[1],nClient);
						if (write(sockmsg,&mex, sizeof(mex))<0) {
							printf("<SERVER-ERR> Errore nella write alla socket (3) %s\n",strerror(errno));
							exit(1);
						}
					}
				}
			}
			//invia info al client
			info(sockmsg);*/
		}
		//condizione d'uscita
		/*if (prenotazione[0]==0 || prenotazione[1]==0)
			break;*/
	}
	close(sockmsg);
	printf("<SERVER-INFO> Server %d: ho chiuso la socket di dialogo con il client %d\n",getpid(),nClient);
}

/*
 * Crea una socket di ascolto e accetta connessioni da parte di più
 * cliente, delegando per ciascun client la gestione ad uno specifico
 * thread detached
 */
int main(int argc, char** argv) {
	int sock; /* socket di ascolto */
	int sockmsg; /* socket di dialogo */
	struct sockaddr_in server;
	//per i thread
	pthread_t thread;
	pthread_attr_t attr;

	//correttezza parametri
	if ( argc != 2 ) {
		printf("<SERVER-ERR> Devi specificare <numero-della-porta>\n");
		exit(EXIT_FAILURE);
	}

	//creao la socket TCP per la famiglia di processi su Internet
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if( sock < 0 ) {
		printf("<SERVER-ERR> server %d: errore %s nel creare la socket\n", getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}

	//inizializzo la struttura del server
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;	//indirizzo della macchina
	server.sin_port = htons(atoi(argv[1]));

	//assegno alla socket "sock" una porta contenuta nella struttura "server"
	if( bind(sock, (struct sockaddr *)&server, sizeof(server))<0 ) {
		printf("<SERVER-ERR> server %d: bind fallita\n", getpid());
		exit(EXIT_FAILURE);
	}
	//inizializzo il cinmea
	inizializzaSala(); //inizializza sala TODO

	printf("<SERVER-INFO> server %d: pronto sulla porta %d\n", getpid(), ntohs(server.sin_port));

	//etichetto la socket come "di ascolto" TODO connessioni max
	if( listen(sock, 4) <0 ) {
		printf("<SERVER-ERR> server %d: errore %s nella listen\n",getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}

	//il server cicla
	while(1) {
		//estraggo una connessione dalla coda, se non c'è rimango in attesa
		if( (sockmsg = accept(sock, 0, 0)) <0 )	{
			printf("<SERVER-ERR> server %d: errore %s nella accept\n",getpid(), strerror(errno));
			exit(1);
		}
		printf("<SERVER-INFO> numero socket %d\n",sockmsg);
		printf("<SERVER-INFO> server %d: accettata una nuova connessione\n",getpid());

		//delego ad un thread detached e vado a gestire la connessione seguente
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //detached
		pthread_create(&thread,&attr,threadPrincipale,(void* )sockmsg);
		pthread_attr_destroy(&attr);
	}
	close(sock);
	printf("<SERVER-INFO> server %d: ho chiuso la socket di ascolto\n",getpid());
}
