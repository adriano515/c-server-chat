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

int sockfd = 0;
char s[INET_ADDRSTRLEN];
   
int sendMsg(int csocket, char *buf, int *len ){

	int total = 0;
	int bytesleft = *len;
	int n;
	while (total < *len) {
		n = send(csocket, buf + total, bytesleft, 0);
		if (n == -1) {
			break;
		}
		total += n;
		bytesleft -= n;
	}
	
	*len = total;
	return n == -1?-1:0; //return -1 on fail, 0 on success
}

void usrMsg(char msg[]){
	
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
			printf("There was an error registering you with username: '%s'", token);
			
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
			printf("La informacion que solicito sobre '%s' es:  IP->'%s', port->'%s', status->'%s'", params5[0] ,params5[1], params5[2], params5[3]);
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
			printf("La lista de usuarios es '%s'", params7[1]);

			return;
		case 8:
			/*
				
			[Mensaje {cliente a servidor}]
				08|emisor|receptor|mensaje
			*/
			return;
		default : 
			//MSG FROM USER
			userMsg(msg);
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

		
	printf("client: connecting to %s\n",s);

	while(1){
		
		if( nbytes = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0) == -1){
			perror ("Error receiving msg");
			exit(1);
		}
		else if(nbytes == 0){
			perror ("Remote host closed connection");
		}
		printf("client:received %s\n", recvBuff);
		protocol = getProt(recvBuff);
		handleResponse(protocol, recvBuff);
	}
}



int main(int argc, char *argv[])
{
    int status = 0, msglen;
    char recvBuff[1024] = " ";
    char msg[1024];
    
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


	msg = "00|YonyB|192.168.5.212|1100|2";
	msglen = strlen(msg);
    if( sendMsg(sockfd, msg, &msglen) == -1){
    	perror("send");
		printf("Only %d bytes were sent due to error \n", msglen);							 
	}

    while(1){
		memset(recvBuff, 0, sizeof recvBuff);
	    
		//revisar el print de abajo, siempre esta devolviendo 2.0.0.0 tiene que ver con la struct de p

		pthread_t read_thread;					
		
		if(pthread_create( &read_thread, NULL, readServer, &sockfd) < 0){
			perror("Could not create thread");
			return 1;
		}

	    
		printf("Enter message : ");
	    scanf("%s" , msg);
	
	    msglen = strlen(msg);
	    if( sendMsg(sockfd, msg, &msglen) == -1){
	    	perror("send");
			printf("Only %d bytes were sent due to error \n", msglen);
								 
	    }
	}	    
    close(sockfd);

    return 0;
}
