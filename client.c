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
void readInfoFromserver(int socket){
	//TODO aggiungere controllo MUTEX
	int i, j, n_file, n_posti, cod, cod_fila, pren;
	//riceve numero file
	if (recv(socket, &(n_file), sizeof(int),0)< 0) {
		printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}
	printf("%d\n",n_file);
	for(i=0; i<n_file; i++){
		//riceve numero posti pre ogni fila
		if (recv(socket, &(n_posti), sizeof(int),0)< 0) {
			printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		printf("%d ",n_posti);
		for(j=0; j<n_posti; j++){
			//INT codice, INT codice_fila,INT prenotato. manda i dati struttura posto
			if (recv(socket, &(cod), sizeof(int),0)< 0) {
				printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
				exit(1);
			}
			if (recv(socket, &(cod_fila), sizeof(int),0)< 0) {
				printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
				exit(1);
			}
			if (recv(socket, &(pren), sizeof(int),0)< 0) {
				printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
				exit(1);
			}
			printf("%d",pren);
		}
		printf("\n");
	}
			//stampa sala? TODO
			//stampaSala(*sala_a);
			printf("\n");
}

int * input_posto(char* s){ //side effect su fila e posto
	char* p=malloc(sizeof(char)*16);
	static int ret[2];
	strcpy(p,s);
	int i =0;
	p = strtok(s, "/");
	printf("[INPUT_POSTO] %s\n", p);
	while(p != NULL) {
    printf("%s\n", p);
		if(i==0) ret[0] = atoi(p);
		else if(i==1) ret[1]=atoi(p);
    p = strtok(NULL, " ");
		i++;
	}
	return ret;
}

void effettuaPrenotazione(int socket){
	struct posto* prenotazione;
	//l'utente inserisce una lista di posti e cerca di prenotarli
	printf("EFFETTUA prenotazione\n");
	int i, n_posti;
	char buf[16];
	printf("Numero di posti da prenotare: ");
	gets(buf);
	n_posti = atoi(buf);
	//printf("%d\n", n_posti);
	//printf("%s\n", buf);
	prenotazione=malloc(sizeof(struct posto)*n_posti);
	/*else{
		printf("non abbastanza posti, bitch\n");
		exit(1);
	}*/
	memset(buf, 0, sizeof(buf));
	int *p = malloc(sizeof(int)*2);
	for(i=0; i<n_posti;i++) {
		//printf("%d\n", i);
		printf("Posto %d : ", i);
		gets(buf);
		p = input_posto(buf);
		printf("[POSTO LETTO] %d/%d \n", p[0],p[1]);
		prenotazione[i].codice_fila = p[0];
		prenotazione[i].codice = p[1];
		prenotazione[i].prenotato=1;
	}
	//manda lunghezza prenotazione
	if (send(socket, &(n_posti), sizeof(int),0)< 0) {
		printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}

	//INT codice, INT codice_fila,INT prenotato. manda l'array prenotazione
	for(i=0; i<n_posti; i++){
		if (send(socket, &(prenotazione[i].codice), sizeof(int),0)< 0) {
			printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		if (send(socket, &(prenotazione[i].codice_fila), sizeof(int),0)< 0) {
			printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		if (send(socket, &(prenotazione[i].prenotato), sizeof(int),0)< 0) {
			printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		//printf("%d",pren);
	}
	//aspetta conferma validità posti
	char* conferma = malloc(sizeof(char)*CODLEN);

	if (recv(socket, conferma, sizeof(char)*CODLEN,0)< 0){
		printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}
	if(strcmp(conferma,NONDIS)!=0){
		printf("\n-----------------ATTENZIONE-------------\n");
		printf("Questo è il codice prenotazione: %s\n\n",conferma);
		//sono validi, conferma contiene codice prenotazione
	}
	else{
		printf("--- Posti NON VALIDI o NON DISPONIBILI! ---\n");
		//è NONDIS, allora non disponibili o non validi
	}

}

void annullaPrenotazione(int socket){
	printf("ANNULLA prenotazione \n");
	printf("Inserire codice prenotazione: \n");
	char cod_pre[CODLEN];
	gets(cod_pre);
	printf("%s\n", cod_pre);
	//leggo e invio cod pren
	if (send(socket, cod_pre, sizeof(char)*CODLEN,0)< 0) {
		printf("[<SERVER-ERR> Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}

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
	int operazione;
	char op[16];
	while(1){
		deb(DEB, "main loop");
		//presentazione
		stampaComandi();
		printf("Scegliere comando\n");
		gets(op);

		//TODO controlli su operazione
		if(send(sock, op , sizeof(op),0) <0) {
			printf("<CLIENT %d - ERR> Errore %s durante la write nella socket\n",getpid(),strerror(errno));
			exit(1);
		}
		operazione = atoi(op);
		switch (operazione) {
			case 1:   //guarda posti disponibili
				deb(DEB, "read from server");
				readInfoFromserver(sock); //legge i dati della sala dal server TODO
				break;
			case 2:   //prenota posti
				effettuaPrenotazione(sock); //TODO effettua prenotazione
				break;
			case 3:
				annullaPrenotazione(sock);  //TODO annulla prenotazione
				break;
			default:
				exit(1);
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
