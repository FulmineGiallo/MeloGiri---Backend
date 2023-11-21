#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include "cJSON.h"
#include <arpa/inet.h>
#define PORT 8080
#define IP_ADDRESS "209.38.244.243"
#define MESSAGE_SIZE 256

pthread_mutex_t mutex;


void trim(char *str);
void trim(char *str)
{
    int len = strlen(str);
    int i = 0;
    while (isspace(str[i])) {  // Rimuovi gli spazi iniziali
        i++;
    }
    if (i > 0) {
        memmove(str, str + i, len - i + 1);  // Sposta la stringa
        len -= i;
    }
    i = len - 1;
    while (i >= 0 && isspace(str[i])) {  // Rimuovi gli spazi finali
        str[i] = '\0';
        i--;
    }
}
char *subString(char *string, char *posIniziale, char *posFinale)
{
    int len = (posFinale - posIniziale) - 1;
    char *subString = malloc(len + 1); // +1 per il carattere di terminazione

    strncpy(subString, posIniziale, len);
    subString[len] = '\0'; // aggiungi il carattere di terminazione

	trim(subString);

    return subString;
}
char *getBevande(MYSQL *conn)
{
	char *fileJSON;

	MYSQL_RES *result;
	MYSQL_ROW row;
	char query[100];
	sprintf(query, "SELECT * FROM bevanda");

	if(mysql_query(conn,query))
	{
		fprintf(stderr, "Errore nell'esecuzione della query: %s\n", mysql_error(conn));
		exit(1);
	}
	result = mysql_store_result(conn);

	if(mysql_num_rows(result)==0)
	{
		printf("Non ci sono bevande disponibili! \n");
		mysql_free_result(result);
		mysql_close(conn);
	}
	else
	{
		printf("Richiesta API getBevande() \n \n");

		cJSON *root = cJSON_CreateArray(); // crea un array JSON per contenere i risultati

		while ((row = mysql_fetch_row(result)) != NULL)
		{
			// Crea un oggetto JSON per la riga corrente
			cJSON *bevanda = cJSON_CreateObject();
			cJSON_AddStringToObject(bevanda, "idbevanda", row[0]);
			cJSON_AddStringToObject(bevanda, "nome", row[1]);
			cJSON_AddStringToObject(bevanda, "photo_url", row[2]);
			cJSON_AddStringToObject(bevanda, "livello_alcolico", row[3]);
			cJSON_AddStringToObject(bevanda, "descrizione", row[4]);
			cJSON_AddStringToObject(bevanda, "categoria", row[5]);
			cJSON_AddStringToObject(bevanda, "prezzo", row[6]);

			// Aggiungi l'oggetto JSON alla fine dell'array
			cJSON_AddItemToArray(root, bevanda);
			}

			fileJSON = cJSON_Print(root); // crea la stringa JSON dall'array completo

		}
		mysql_free_result(result);
		return fileJSON;
}

//creo un thread per la gestione di connessione e query mysql. questo script andrà inserito all'interno della socket del server
void *services(void *arg)
{
	char buffer[10000];
	char sceltaStart[1000];
   	int bytesRecv = 0; // Indicatori di errore di ricezione

	MYSQL *conn; //oggetto per stabilire la connessione
	MYSQL_RES *res; //oggetto per i scorrere i risultati delle query
	//MYSQL_ROW row; oggetto per individuare le righe delle tabelle
	
	char *server="database"; 	//host a cui connettersi IP_privato
	char *username="root"; 		//user db
	char *password="root"; 		//password db
	char *database="melogiri"; 	//nomedb
	char query[1000];

	conn = mysql_init(NULL);

	if(!mysql_real_connect(conn,server,username,password,database,0,NULL,0))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	else
		printf("Connessione avvenuta al DATABASE \n");

	int newSocket = *((int *)arg);
	// Riceve messaggio Scelta login dal client
    bytesRecv = 0;
    do
    {
        bytesRecv = recv(newSocket, (char *)sceltaStart, sizeof(sceltaStart), 0);
        if (bytesRecv == -1)
        {
            printf( "\nConnection Closed.\n");
            break;
        }
        if (bytesRecv < 1) {

            printf("\nRicevuto 0 Chiusura Thread in corso\n");
            /* close connection DB */
            mysql_free_result(res);
            mysql_close(conn);


            /* close connection Socket */
            printf("\nCLIENT DISCONNESSO\n");
            printf("Exit socketThread \n");
            close(newSocket);
            pthread_exit(NULL);

            return 0;
        }
    }
    while(bytesRecv == 0);

	if(sceltaStart[0]=='1') //API LOGIN
	{
		//FORMATO RICHIESTA: 1carminefb@live.it&$password
		char email[100];
		char pswd[100];
		char emailFormattata[100];
		char passwordFormattata[100];

		printf("EMAIL + PASSWORD : %s \n", sceltaStart);

		//RECUPERO EMAIL da SceltaStart
		char *posEmail = strchr(sceltaStart, '&');
		int len = posEmail - sceltaStart;
		strncpy(email, sceltaStart, len);
		strcpy(emailFormattata, email + 1);


		//RECUPERO PASSWORD da SceltaStart
		int lenLogin = strlen(sceltaStart) - 1;
		char *posPassword = strchr(sceltaStart, '$');

		int lenPassword = (posPassword - sceltaStart) + 1;
		strcpy(passwordFormattata, posPassword + 1);

		trim(passwordFormattata);
		sprintf(query, "SELECT * FROM utente WHERE email = '%s' && password = '%s';", emailFormattata,passwordFormattata);


		//verifico credenziali di accesso
		MYSQL_RES *result;
		MYSQL_ROW row;
		char *fileJSON;

		if(mysql_query(conn,query))
		{
			fprintf(stderr, "Errore nell'esecuzione della query: %s\n", mysql_error(conn));
			exit(1);
		}
		result = mysql_store_result(conn);

		if(mysql_num_rows(result)==0)
		{
			printf("Credenziali di accesso non valide \n");
			mysql_free_result(result);
			mysql_close(conn);
			send(newSocket,"USER_NOTFOUND",13,0);
		}
		else
		{
			printf("Accesso consentito\n");

			while((row = mysql_fetch_row(result)) != NULL)
			{
				// Creazione dell'oggetto JSON
				cJSON *root = cJSON_CreateObject();
				cJSON_AddStringToObject(root, "idutente", row[0]);
				cJSON_AddStringToObject(root, "nome", row[1]);
				cJSON_AddStringToObject(root, "cognome", row[2]);
				cJSON_AddStringToObject(root, "data_di_nascita", row[3]);
				cJSON_AddStringToObject(root, "email", row[4]);
				cJSON_AddStringToObject(root, "password", row[5]);
				char *json_str = cJSON_Print(root);

				send(newSocket, json_str, strlen(json_str), 0);
				// Liberazione della memoria dell'oggetto JSON e della stringa JSON
				cJSON_Delete(root);
				free(json_str);
			}
			//send(newSocket, fileJSON, strlen(fileJSON), 0);

			mysql_free_result(result);
		}
	}
	if(sceltaStart[0]=='2') //API REGISTER
	{
		/*
		Il Client manda la sequenza: nome_cognome&data?email!password
		*/
		char *nome;
		char *cognome;
		char *data;
		char *email;
		char *password;

		//LUNGHEZZA DI TUTTO IL MESSAGGIO INVIATO
		int lenRegister = strlen(sceltaStart) - 1;

		//Recupero Nome
		nome = subString(strchr(sceltaStart, '2') + 1, strchr(sceltaStart, '2') + 1, strchr(sceltaStart, '_') + 1);

		//RECUPERO COGNOME
		cognome = subString(strchr(sceltaStart, '_') + 1, strchr(sceltaStart, '_') + 1, strchr(sceltaStart, '&') + 1);

		//RECUPERO Data
		data = subString(strchr(sceltaStart, '&') + 1, strchr(sceltaStart, '&') + 1, strchr(sceltaStart, '?') + 1);

		//RECUPERO Email
		email = subString(strchr(sceltaStart, '?') + 1, strchr(sceltaStart, '?') + 1, strchr(sceltaStart, '!') + 1);

		//RECUPERO Password
		password = subString(strchr(sceltaStart, '!') + 1, strchr(sceltaStart, '!') + 1, strchr(sceltaStart, '\0') + 1);


		printf("NOME: %s \n", nome);
		printf("Cognome: %s \n", cognome);
		printf("Data: %s \n", data);
		printf("Email: %s \n", email);
		printf("Password: %s \n", password);

		char query[1000];
	    	sprintf(query, "INSERT INTO utente (email, password, nome, cognome, data_nascita) VALUES ('%s', '%s', '%s', '%s', '%s');", email, password, nome, cognome, data);

		if (mysql_query(conn, query))
		{
			fprintf(stderr, "Errore nell'esecuzione della query: %s\n", mysql_error(conn));
			exit(1);
		}

		free(nome);
		free(cognome);
		free(data);
		free(email);
		free(password);
	  	printf("Utente registrato con successo!\n");

		send(newSocket,"Registration_Successful",23,0);

	}
	if(sceltaStart[0]=='3') //QUERY PER TUTTE LE BEVANDE disponibili nel DATABASE
	{
		//GENERO LE BEVANDE
		char *fileJSON = getBevande(conn);
	
		send(newSocket, fileJSON, strlen(fileJSON), 0);

		//libero la memoria
		free(fileJSON);
	}
	if(sceltaStart[0]=='4') //CARRELLO
	{
		//Messaggio dal Cliente
		//idUtente!{idBevanda, quantità}, .. , {idBevanda,quantità}$tot_prezzo;

		int fk_utente;
		double tot_prezzo;

		//recupero id utente con funzione substring
		fk_utente = (int) strtol(subString(strchr(sceltaStart, '4')+1, strchr(sceltaStart, '4') + 1, strchr (sceltaStart, '!') +1), (char **)NULL, 10);

		//recupero tot_prezzo con funzione substring
		tot_prezzo = strtod(subString(strchr(sceltaStart, '$'), strchr(sceltaStart, '$') + 1, strchr(sceltaStart, '\0') + 1), NULL);

		char query[1000];

		//prima query= insert ordine
		//seconda query= insert carrello da ordine


		//query ordine
		sprintf(query, "INSERT INTO ordine (stato, data_ordine,fk_utente,tot_prezzo) VALUES ('%s',NOW(),'%d','%f');", "confermato", fk_utente, tot_prezzo);

		if (mysql_query(conn, query))
		{
				fprintf(stderr, "Errore nell'esecuzione della query: %s\n", mysql_error(conn));
				exit(1);
		}

		// recupero l'id dell'ordine appena inserito
		int id_ordine = mysql_insert_id(conn);

		printf("Ordine registrato con id %d\n", id_ordine);

		char *bevande=sceltaStart+1;
		char *p = bevande;
		char *sub; //sottostringa risultante
		char *idBevandaP;
		char *quantitaBevanda;

		while ((p = strchr(p, '{')) != NULL)
		{

				sub = subString(bevande, p + 1, strchr(p, '}') + 1);


				printf("%s \n", sub);
				idBevandaP = subString(sub, sub, strchr(sub, ',') + 1);
				quantitaBevanda = subString(sub,strchr(sub,',') + 1, strchr(sub, '\0') + 1);
				/*
				printf("ID: %s \n", idBevandaP);
				printf("QUANTITA: %s \n", quantitaBevanda);
				*/
				char query[1000];

				sprintf(query, "INSERT INTO carrello (fk_ordine, fk_bevanda, quantita_bevanda) VALUES (%d, %d, %d)",id_ordine, atoi(idBevandaP), atoi(quantitaBevanda));
				if (mysql_query(conn, query))
				{
						fprintf(stderr, "Errore nell'esecuzione della query: %s\n", mysql_error(conn));
						exit(1);
				}
				free(sub);
				p++;
		}

		free(idBevandaP);
		free(quantitaBevanda);
		send(newSocket,"Ordine_OKKE",11,0);

	}

	//CHIUDO CONNESSIONE
	mysql_close(conn);
	//CHIUDO IL THREAD
	pthread_exit(NULL);
}

int main()
{
    int server_fd;
	int client_fd;
	int valread;
	struct sockaddr_in address;
	int opt=1;
	int addrlen=sizeof(address);

	char buffer[MESSAGE_SIZE]={0};
	char *hello = "Ciao client Java!";


	//creo il socket file descriptor
	if((server_fd=socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("Errore nella creazione del socket");
		exit(EXIT_FAILURE);
	}
	
	printf("Benvenuto nel SERVER ... provo a connettermi al dabatabase in locale \n");
	printf("Sto aspettando richieste... ");

	//setto opzioni del socket
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("Errore nel settaggio delle opzioni del socket");
		exit(EXIT_FAILURE);
	}

	// Impostazione dell'indirizzo
	memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

	//Bind del socket
	if(bind(server_fd,(struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("Errore nel binding del socket");
		exit(EXIT_FAILURE);
	}
	//metto in ascolto
	if(listen(server_fd, 5) < 0)
	{
		perror("Errore nell ascolto delle connessioni in entrata");
		exit(EXIT_FAILURE);
	}
	
	pthread_mutex_init(&mutex, NULL);
	pthread_t tid;
	int res;
	
	while(1)
	{
		//accetto connessione in entrata
		
		if((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror("Errore nell'accettazione della connessione in entrata");
			exit(EXIT_FAILURE);
		}

		pthread_mutex_lock(&mutex);
		res=pthread_create(&tid,NULL,services,&client_fd);

		if(res)
		{
			printf("Errore creating thread: %d\n", res);
			return 1;
		}

		pthread_join(tid,NULL);
		pthread_mutex_unlock(&mutex);
	}

 	close(client_fd);
	close(server_fd);
	return 0;
}
