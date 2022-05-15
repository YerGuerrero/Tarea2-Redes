/*
	C socket server example
*/

#include <stdio.h>
#include <string.h>	//strlen
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include <regex.h>
#include <limits.h>


unsigned char ip_hex[4];
unsigned char mask_hex[4];
unsigned char result_network[4];
unsigned char result_brodcast[4];
unsigned char *mascaraBits;

void transformIPHex(unsigned char *cadena){
	for (int i = 0; i < 3; i++)
	{
		ip_hex[i]=0;
	}
	int num= 0;
	int lenCadena= strlen(cadena); 
	int multiplicador= 1;
	int indexIP = 3;
	char copiaCadena[strlen(cadena)];
    strcpy(copiaCadena,cadena);
	
	while (lenCadena>0)
	{
		lenCadena--;
		if (copiaCadena[lenCadena]== 0x2E)
		{
			ip_hex[indexIP]=num;
			num=0;
			multiplicador=1;
			indexIP--;
		}else{	
			num= num + ((cadena[lenCadena]) - 0x30) * multiplicador;
			multiplicador=multiplicador*10;
		}
	}
	ip_hex[indexIP]=num;
}

void transformMaskHex(unsigned char *cadena){
	for (int i = 0; i < 3; i++)
	{
		mask_hex[i]=0;
	}
	int num= 0;
	int lenCadena= strlen(cadena); 
	int multiplicador= 1;
	int indexMask = 3;
	char copiaCadena[strlen(cadena)];
    strcpy(copiaCadena,cadena);
	if (lenCadena>15){
		lenCadena=15;
	}
	while (lenCadena>0)
	{
		lenCadena--;
		if (copiaCadena[lenCadena]== 0x2E)
		{
			mask_hex[indexMask]=num;
			num=0;
			multiplicador=1;
			indexMask--;
		}else{	
			num= num + ((cadena[lenCadena]) - 0x30) * multiplicador;
			multiplicador=multiplicador*10;
		}
	}
	mask_hex[indexMask]=num;
}

void brodcast(unsigned char *ip, unsigned char *mask){
	transformIPHex(ip);
	transformMaskHex(mask);
	for (int i = 0; i < 4; i++)
	{
		result_brodcast[i]= ip_hex[i] | (~mask_hex[i]);
	}
	printf("RED BRODCAST: %i.%i.%i.%i\n", result_brodcast[0], result_brodcast[1], result_brodcast[2], result_brodcast[3]);
	puts("Entra a la función brodcast");
}

void network(unsigned char *ip, unsigned char *mask){
	transformIPHex(ip);
	transformMaskHex(mask);
	for (int i = 0; i < 4; i++)
	{
		result_network[i]= ip_hex[i] & mask_hex[i]; 
	}
	printf("NETWORK ORIGEN: %i.%i.%i.%i\n", result_network[0], result_network[1], result_network[2], result_network[3]);
	puts("Entra a la función metwork");
}

void hostRange(unsigned char *ip, unsigned char *mask){
	network(ip,mask);
	brodcast(ip, mask);
	printf("HOSTS RANGE: %i.%i.%i.%i - %i.%i.%i.%i\n", result_network[0], result_network[1], result_network[2], result_network[3]+1,result_brodcast[0], result_brodcast[1], result_brodcast[2], result_brodcast[3]-1);
	puts("Entra a la función hostRange");
}

char* obtenerDato(char* request, int pos){
    char copiaRequest[1024];
    strcpy(copiaRequest,request);
    char *ptr = strtok(copiaRequest, " ");
    for (int i=1;i<pos;i++){
        ptr = strtok(NULL, " ");
    }
    return ptr;
}

char *ConvertirAInt(char* binario){ 
  int res = (int) strtol(binario, NULL, 2); 
  char *str = calloc(8, sizeof(char)); 
  sprintf(str, "%i", res); 
  printf("%s \n", str); 
  return str; 
 
}

void convertirMaskBits(char* mask){
	char copiaRequest[5];
	strcpy(copiaRequest,mask);
	char *ptr = strtok(copiaRequest, "/");
	int numDez= atoi(ptr);
	int contBits=1;
	unsigned char *parte = calloc(8, sizeof(unsigned char));
	mascaraBits = calloc(36, sizeof(unsigned char));
	for (int i = 0; i < 32; i++)
	{
		if (numDez>0){
			strcat(parte, "1"); 
			numDez--;
		}
		else{
			strcat(parte, "0");
		}
		
		if(contBits==8||contBits==16||contBits==24){
			strcat(mascaraBits, ConvertirAInt(parte));
			strcat(mascaraBits, ".");	
			parte = calloc(8, sizeof(unsigned char));
		}
		if(contBits==32){
			strcat(mascaraBits, ConvertirAInt(parte));
		}
		contBits++;
	}
}

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];
	regex_t reegexBrodcast;
    regex_t reegexNetwork;
    regex_t reegexHostRange;
    regex_t reegexRandomSubnet;
	regex_t reegexMaskBits;

    regcomp( &reegexBrodcast, "GET BRODCAST", 0);
    regcomp( &reegexNetwork, "GET NETWORK NUMBER", 0); // ip & mask -> inicio de red
    regcomp( &reegexHostRange, "GET HOSTS RANGE", 0);
    regcomp( &reegexRandomSubnet, "GET RANDOM SUBNETS NETWORK", 0);
	regcomp( &reegexMaskBits, "[/0-9]+", REG_EXTENDED);


	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Socket no creado");
	}
	puts("Socket creado");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 9666 );
	
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("Error en bind");
		return 1;
	}
	puts("bind terminado");
	
	listen(socket_desc , 3);
	
	puts("Esperando por conexiones entrantes...");
	c = sizeof(struct sockaddr_in);

	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("Cliente no aceptado");
		return 1;
	}
	puts("Cliente aceptada");
	
	while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
	{
		printf("He recibido: %s", client_message);
        int regexBrodcast= regexec(&reegexBrodcast, client_message, 0, NULL, 0);
        int regexNetwork= regexec(&reegexNetwork, client_message, 0, NULL, 0);
        int regexHostRange= regexec(&reegexHostRange, client_message, 0, NULL, 0);
        int regexRandomSubnet= regexec(&reegexRandomSubnet, client_message, 0, NULL, 0);
        if(regexBrodcast==0){
			unsigned char *ip= obtenerDato(client_message,4);
			printf("IP: %s\n",ip);
			unsigned char *mask = obtenerDato(client_message,6);
			printf("Mask: %s\n",mask);
			if(strlen(mask)<=5){
				convertirMaskBits(mask);
				for (int i = 0; i < 3; i++)
				{
					mask_hex[i]=0;
				}
				mask=mascaraBits;
			}
            brodcast(ip, mask);
			char *msj= calloc(36, sizeof(char));
  			sprintf(msj, "%i.%i.%i.%i\n", result_brodcast[0],result_brodcast[1],result_brodcast[2],result_brodcast[3]); 
			write(client_sock , msj , strlen(msj));
        }else if(regexNetwork==0){
			unsigned char *ip= obtenerDato(client_message,5);
			printf("IP: %s\n",ip);
			unsigned char *mask= obtenerDato(client_message,7);
			printf("Mask: %s\n",mask);
			if(strlen(mask)<=5){
				convertirMaskBits(mask);
				for (int i = 0; i < 3; i++)
				{
					mask_hex[i]=0;
				}
				mask=mascaraBits;
			}
			network(ip, mask);
			char *msj= calloc(36, sizeof(char)); 
  			sprintf(msj, "%i.%i.%i.%i\n", result_network[0],result_network[1],result_network[2],result_network[3]); 
			write(client_sock , msj , strlen(msj));
        }else if(regexHostRange==0){
			unsigned char *ip= obtenerDato(client_message,5);
			printf("IP: %s\n",ip);
			unsigned char *mask= obtenerDato(client_message,7);
			printf("Mask: %s\n",mask);
			if(strlen(mask)<=5){
				convertirMaskBits(mask);
				for (int i = 0; i < 3; i++)
				{
					mask_hex[i]=0;
				}
				mask=mascaraBits;
			}
			hostRange(ip, mask);
			char *msj= calloc(36, sizeof(char)); 
  			sprintf(msj, "%i.%i.%i.%i - %i.%i.%i.%i\n", result_network[0],result_network[1],result_network[2],result_network[3]+1, result_brodcast[0],result_brodcast[1],result_brodcast[2],result_brodcast[3]-1); 
			write(client_sock , msj , strlen(msj));
        }else if(regexRandomSubnet==0){
			//No se implementa
        }else{
            puts("Comando incorrecto");
        }
		//Send the message back to client
		//write(client_sock , client_message , strlen(client_message));
	}
	
	if(read_size == 0)
	{
		puts("CLiente desconectado");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
	
	return 0;
}