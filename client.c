#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <time.h>

int sockfd = 0;
char s[INET_ADDRSTRLEN];
char *user;

int sendMsg( char *buf, int *len){
	int total = 0;
	int bytesleft = *len;
	int n;
	while(total<*len){
		n = send(sockfd, buf+ total, bytesleft, 0);
		if(n==-1){
			printf("Error sending message\n", buf);
			fflush(stdout);
			break;		
		}
	printf("Sent msg '%s' to server\n", buf);
	fflush(stdout);
	*len = total;
	return n == -1?-1:0;
	}
}


char* scanInput(){
	
	char *name = malloc(1024);
	if (name == NULL){
		printf("No memory\n");
		return;
	}
	fgets(name, 1024, stdin);
	
	if((strlen(name) > 0) && (name[strlen (name) - 1] == '\n'))
		name[strlen (name) - 1] = '\0';
	return name;
	
}

void usrMsg(char msg[]){
		
}

void CambiarEstado(char *estado, char *actividad){
	
	char msg[1024] = "03|";
	int msglen;
	strcat(msg,user);
	strcat(msg,"|");
	strcat(msg,estado);
	msglen = strlen(msg);
	if( sendMsg( msg, &msglen) == -1){
		printf("Error %s\n", actividad);
		perror("Error\n");				 
	}
	
	return;
}

void changeStatus(){
		int opcion;
		printf("\n  1. Activo");
		printf("\n  2. Iddle");
		printf("\n  3. Away");
		opcion = atoi(scanInput());
		switch(opcion){
		case 1 :
				CambiarEstado("0","Opcion2");
				return;
		case 2 :
				CambiarEstado("1","Opcion2");
				return;
		case 3 :
				CambiarEstado("2","Opcion2");
				return;
		}
}

void *timeOut(){
	
	clock_t t,ts;
	int seconds =0;
	for(;;){
		if((t=clock())>=ts){
			++seconds;
			
			if(seconds==30){
				
				CambiarEstado("1","Idle");
			}	
			ts = t + CLOCKS_PER_SEC;	
		}
	}
	
	return;
}

void InformacionUsuario(){
	char msg[1024] = "04|";
	int msglen;
	char *userInformation;
	printf("Insert user to ask information\n");
	userInformation = scanInput();
	strcat(msg,userInformation);
	strcat(msg,"|");
	strcat(msg,user);
	
	msglen = strlen(msg);
	if( sendMsg( msg, &msglen) == -1){
		perror("Error in protocol 04 InformacionUsuario");					 
	}
}

void handleResponse(int protocol, char msge[]){
	
	//msge = "00|user|192.168.0.1|1100|0";
	char delim[1] = "|";
	char *token;
	int i;
	char *msg = strdup(msge);
	//protocol = 8;
	
	
	token = strtok(msg, delim);
	
	switch(protocol){
		case 0 :
			/*
			Registro de Usuario
				[Registro {cliente a servidor}]
				00|ususario|direccionIP|puerto|status¬
			*/
			
			return;
		case 1 :
			/*
			[Error de registro {servidor a cliente}]
				01|usuario|direccionIP¬
				implemented in regUser function by calling errorReg function
			*/
			printf("There was an error registering you with username: '%s'\n", token);
			
			return;
		case 2 :
			/*
			
			Liberacion de Usuario
			[Cliente cierra chat {cliente a servidor}]
				02|usuario¬

			*/
			return;
		case 3 :
			/*
			
			Cambio de status
			[Cliente cambio estado {cliente a servidor}]
				03|usuario|status¬
			*/
			return;
		case 4 :
			/*
			     
			Obtencion de informacion de usuario
			[Peticion de informacion {cliente a servidor}]
				04|usuarioDeInfo|usuarioQuePide¬
			*/
			return;
		case 5 :
			/*			
			[Retorno de informacion {servidor a cliente}]
				05|usuario|direccionIP|puerto|status¬
			*/
			i = 0;
			char *params5[4]; 
			while ((token = strtok(NULL, delim)) != NULL){
				
				params5[i] = token;
				i ++;
			}
			printf("La informacion que solicito sobre '%s' es:  IP->'%s', port->'%s', status->'%s'\n", params5[0] ,params5[1], params5[2], params5[3]);
			return;
		case 6 :
			/*
			
			Listado de usuarios conectados
			[Peticion de listado {cliente a servidor}]
				06|usuarioQuePide¬
			*/
			
			return;
		case 7 :
			/*

			[Retorno de listado {servidor a cliente}]
				07|usuarioQuePide|usuario1+estado1&usuario2+estado2...usuarioN+estadoN¬
			*/
			
			i = 0;
			char *params7[4]; 
			while ((token = strtok(NULL, delim)) != NULL){
				
				params7[i] = token;
				i ++;
			}
			printf("La lista de usuarios es '%s'\n", params7[1]);
			fflush(stdout);
			return;
		case 8:
			/*
				
			[Mensaje {cliente a servidor}]
				08|emisor|receptor|mensaje
			*/
			return;
		default : 
			//MSG FROM USER
			usrMsg(msg);
			return;	
	}

}

int getProt(char msg[]){
	
	//msg = "00|user|192.168.0.1|1100|0";
	
	long protocol;
	char *str;
	protocol = strtol(msg, &str, 10);
	
	if (protocol){
		return (int)protocol;
	}
	else if(msg != str){
		return 0;
	}

	return 9;
}




void *readServer(void *arg){

	char recvBuff[1024] = " ";
 	int nbytes = 0;
	int protocol;

	while(1){
		memset(recvBuff, 0, sizeof recvBuff);
		if( nbytes = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0) == -1){
			perror ("Error receiving msg");
			exit(1);
		}
		else if(nbytes == 0){
			//perror ("Remote host closed connection");
		}
		else{
			printf("Recived message '%s'\n", recvBuff);		
		}
		printf("client:received %s\n", recvBuff);
		fflush(stdout);
		protocol = getProt(recvBuff);
		handleResponse(protocol, recvBuff);
	}
}


int cliente(int argc, char *argv[]){
	char msg[1024];
	char *msg2;
	int test;
	int msglen;
    while(1){
		
		memset(msg, 0, sizeof msg);
		memset(msg2, 0, sizeof msg2);	
		strcat(msg, "08|");			
		strcat(msg, user);
		strcat(msg, "|");
		//cambiar estado a activo
		CambiarEstado("0","Activo");
		char *userToSend;
		printf("Enter user to send message : \n");
		userToSend = scanInput();
	   
	    	strcat(msg, userToSend);
		strcat(msg, "|");
			
		
		pthread_t timeout;
		pthread_create(&timeout,NULL,timeOut,NULL);
		
		fprintf(stdout, "Enter message : \n");
		msg2 = scanInput();
	    	//scanf("%s", msg);
	   
		strcat(msg, msg2);
				
		pthread_cancel(timeout);
		//fprintf(test, "%d" ,msg2);
		if(test == 3){
			fprintf(stdout, msg);
			printf("you are out of the chat\n");
			break;
		}
		
	    msglen = strlen(msg);
	    if( sendMsg(msg, &msglen) == -1){
	    	perror("send");
			printf("Only %d bytes were sent due to error \n", msglen);
								 
	    }
	}	    
    close(sockfd);
	CambiarEstado("2","Away");
    return;
}

void Ayuda(void){
	printf("\n  here is the help\n");	
	return;
}

void ListarUsuarios(void){
	char msg[1024] = "06|";
	int msglen;
/*
	char* token;
	char* token2;
	char* string;
	char* tofree;
	string = strdup("usuario1+estado1&usuario2+estado2&usuarioN+estadoN");
	if(string!=NULL){
		tofree = string;
		while((token=strsep(&string,"&"))){
		printf("\n******************\n");
			while((token2=strsep(&token,"+"))){
				printf("%s \n", token2);		
			}	
		printf("\n******************");	
		}	
	
	}*/
	strcat(msg, user);
	
	msglen = strlen(msg);
   	if( sendMsg( msg, &msglen) == -1){
    		perror("send");
		printf("Only %d bytes were sent due to error \n", msglen);							 
	}	
	
	return;
}

void Menu(int argc, char *argv[]){
	int opcion;
	char *option;
	do{	
		
		printf("\n  1. Chat");
		printf("\n  2. Change State");
		printf("\n  3. Users List");
		printf("\n  4. HELP");
		printf("\n  5. Ask for users information");
		printf("\n  6. End");
		printf("\n  Choose an option (1-6)");
		opcion = atoi(scanInput());
		
	switch(opcion){
		case 1:cliente(argc,argv);
		break;
		case 2:changeStatus();
		break;
		case 3:ListarUsuarios();
		break;
		case 4:Ayuda();
		break;
		case 5:InformacionUsuario();
		break;
		}
	}
	while(opcion!=6);
	char msg[1024] = "02|";
	int msglen;
	strcat(msg,user);
	msglen = strlen(msg);
	if( sendMsg( msg, &msglen) == -1){
		perror("Error in close");					 
	}
	return;
}

int main(int argc, char*argv[]){
	
	int status = 0, msglen;
    
    char msg[1024] = "00|";
	
	printf("Insert user: \n");
	user = scanInput();
    strcat(msg, user);
	strcat(msg, "|192.168.0.1|1100|2");

    struct addrinfo serverinfo, *result, *p;
	
    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> <port of server>\n",argv[0]);
        return 1;
    } 
    memset(&serverinfo, 0, sizeof serverinfo);
    serverinfo.ai_family = AF_INET;
    serverinfo.ai_socktype = SOCK_STREAM;
	
    if(status = getaddrinfo(argv[1], argv[2], &serverinfo, &result) != 0){
		 
    	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
	
    for (p = result; p != NULL; p = p->ai_next){
		
	    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
	    {
			perror("\n Error : Could not create socket \n");
		continue;
	    } 

	    if( connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
	    {
	       close(sockfd);
	       printf("\n Error : Connect Failed \n");
	       continue;
	    } 
	    break;
    }
	
    if (p == NULL){
    	fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
	
	freeaddrinfo(result);  
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)p;

	inet_ntop(p->ai_family, &(ipv4->sin_addr), s, sizeof s);
	printf("client: connecting to %s\n",s);

	msglen = strlen(msg);
	if( sendMsg( msg, &msglen) == -1){
		perror("Error in send");					 
	}

	pthread_t read_thread;					

	if(pthread_create( &read_thread, NULL, readServer, &sockfd) < 0){
		perror("Could not create thread");
		return 1;
	}


	Menu(argc,argv);
	return 0;
}


