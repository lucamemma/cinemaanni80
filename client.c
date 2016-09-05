#include <stdio.h>       /* per usare  input-output */
#include <stdlib.h>      /* per EXIT_FAILURE */
#include <sys/socket.h>  /* per usare socket */
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>        /* gestione degli errori */
#include <unistd.h>

#include "gestioneSala.h"


//stampa i comandi disponibili per l'utente
void stampaComandi(){
	 printf("\n");
	 printf("********************** <CLIENT %d> **********************\n",getpid());
	 printf("Menu' dei comandi:\n");
     printf("-- 1 Visualizza mappa sala cinema;\n");
		 printf("-- 2 Prenota posti;\n");
		 printf("-- 3 Annulla preotazione.\n");
     printf("-- Premi 0 o un carattere alfanumerico per uscire\n");
     printf("\n");
}

//legge dal server le informazioni con la descrizione
void readInfoFromserver(int sock){
	 int i; //contatore
	 struct sala_cinema sala_a;
     //lettura
     if( read(sock, &sala_a, sizeof(sala_a)) < 0 ) {
     	printf("<CLIENT %d - ERR> Errore %s durante la read dalla socket\n",getpid(),strerror(errno));
			exit(1);
     }
     //stampa sala? TODO
		 stampaSala(sala_a);
     printf("\n");
}

//acquisisce l'input sulle prenotazioni
//da standard input nel caso di client normale (mode=1)
//predefinito nel caso di test concreto (mode=2)
/*void prendiPrenotazione(int mode, int prenotazione[2]){
	//parametro locale
	int nPosti;
	//prendo l'input dell'utente
	printf("<CLIENT %d - INPUT> Inserire il numero di posti da prenotare:\n",getpid());
	scanf("%s",&nPosti);
	prenotazione[0] = atoi(lettura);//se inserisce lettere legge 0
	//condizione d'uscita
	//leggo il numero di posti adiacenti
	printf("<CLIENT %d - INPUT> Inserire il numero di posti adiacenti desiderati:\n",getpid());
	scanf("%s",&lettura);
	prenotazione[1] = atoi(lettura);
			//test concreto
}*/

void effettuaPrenotazione(){
	//l'utente inserisce una lista di posti e cerca di prenotarli
	printf("EFFETTUA prenotazione\n");
}

void annullaPrenotazione(){
	printf("ANNULLA prenotazione \n");
}

//processo client
int main(int argc, char** argv) {


	//parametri per usare le socket
	int sock;
	struct sockaddr_in server;
	struct hostent *hp;
	//per colloquiare col server
	char result[256];

	//controllo i parametri
	if(argc!=3) {
		printf("Devi specificare: <host> <numero-della-porta>\n");
		exit(1);
	}

	// chiedo al resolver di tradurre il nome in indirizzo ip
	//se argv[1] e’ gia’ un indirizzo, allora hp viene semplicemente aggiornato con l’indirizzo fornito
	hp = gethostbyname(argv[1]);
	if( hp == NULL ) {
		printf("<ERR> Host %s sconosciuto.\n", argv[1]);
		exit(1);
	}
	//richiedo la socket al sistema operativo
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if( sock < 0 ) {
		printf("<CLIENT %d - ERR> Errore %s nella creazione della socket\n",getpid(),strerror(errno));
		exit(1);
	}
	server.sin_family = AF_INET;
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));

	// connessione
	if( connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0 ) {
		printf("<CLIENT %d - ERR> Errore %s durante la connect\n",getpid(),strerror(errno));
		exit(1);
	}
	printf("<CLIENT %d - INFO> Numero socket %d\n",getpid(),sock);
	printf("<CLIENT %d - INFO> Connesso a %s, porta %d\n",getpid(),argv[1],ntohs(server.sin_port));
	//messaggio benvenuto
	char* buffer = malloc(sizeof(char)*256);int n;
   n = recv( sock,buffer,255,0);
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
	 printf("%s\n", buffer);

	int operazione;
	while(1){
		deb(DEB, "main loop");
		//presentazione
		stampaComandi();
		printf("Scegliere comando\n");
		scanf("%d\n", &operazione); //not safe

		//TODO controlli su operazione
		if( send(sock, &operazione, sizeof(operazione),0) <0 ) {
			printf("<CLIENT %d - ERR> Errore %s durante la write nella socket\n",getpid(),strerror(errno));
			exit(1);
		}
		switch (operazione) {
			case 1:   //guarda posti disponibili
				deb(DEB, "read from server");
				//readInfoFromserver(sock); //legge i dati della sala dal server TODO
				break;
			case 2:   //prenota posti
				effettuaPrenotazione(); //TODO effettua prenotazione
				break;
			case 3:
				annullaPrenotazione();  //TODO annulla prenotazione
				break;
		}
		/*
		//acquisisco l'input
		prendiPrenotazione(mode, prenotazione);

		//condizione d'uscita
		if (prenotazione[0]==0)
			break;

		//scelta del settore e del numero di posti adiacenti
		if( write(sock,&prenotazione, sizeof(prenotazione) ) <0 ) {
			printf("<CLIENT %d - ERR> Errore %s durante la write nella socket\n",getpid(),strerror(errno));
			exit(1);
		}
		else {
			//leggo il risultato dal server
			if( read(sock,&result, sizeof(result)) < 0 )	 {
				printf("<CLIENT %d - ERR> Errore %s durante la read dalla socket\n",getpid(),strerror(errno));
				exit(1);
			}
			printf("\n----> <CLIENT %d - INFO>\n%s\n",getpid(),result);
		}
		//leggo la nuova situazione dei settori
		readInfoFromserver(sock);*/
	}
	close(sock);
	printf("<CLIENT %d - INFO> Grazie e Buona Visione!!!\n",getpid());
	exit(0);
}
