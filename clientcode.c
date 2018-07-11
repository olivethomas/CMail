/****************** CLIENT CODE ****************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

int get_ip(char * hostname , char* ip) 
{  struct hostent *he;     
   struct in_addr **addr_list;     
   int i;     
   if ( (he = gethostbyname( hostname ) ) == NULL)     
   { herror("gethostbyname");         
     return 1;}     
   addr_list = (struct in_addr **) he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
    {   strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;}
    return 1;
}


int main(int argc, char **argv){
  int clientSocket;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;
  
  char *hostname = argv[1];     
  char ip[100];     
  get_ip(hostname , ip);     
  printf("%s resolved to %s\n" , hostname , ip);
  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(atoi(argv[2]));
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr(ip);
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Connect the socket to the server using the address struct ----*/
  addr_size = sizeof serverAddr;
	
  char ch[5],un[50],unto[50],pass[20],msg[1000],fm[1200],buffer[1024];

  if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)!=-1){
  strcpy(fm,"");
  printf("Enter username: ");
  gets(un);
  printf("Enter password: ");
  gets(pass);
  
  send(clientSocket,un,50,0);
  send(clientSocket,pass,20,0);
  recv(clientSocket, buffer, 1024, 0);
  printf("%s\n\n",buffer);   

  if(strcmp(buffer,"Login successful")==0 || strcmp(buffer,"User registered")==0){
    
    printf("1.Send mail\n");
    printf("2.View inbox\n");
    printf("3.Logout\n");
    //do{
    gets(ch);
    send(clientSocket,ch,5,0);
    if(strcmp(ch,"1")==0){
      printf("Send to: ");
      gets(unto);
      send(clientSocket,unto,1024,0);
      printf("Message: ");
      gets(msg);
      strcpy(fm,"\nFrom: ");
      strcat(fm,un);
      strcat(fm,"\nTo: ");
      strcat(fm,unto);
      strcat(fm,"\nMessage: ");
      strcat(fm,msg);
      strcat(fm,"\n");
      printf("%s\n",fm);
      send(clientSocket,fm,1200,0);
      recv(clientSocket, buffer, 1024, 0);
      printf("%s\n",buffer); 
    }
    else if(strcmp(ch,"2")==0){
      printf("Loading inbox...\n");
      recv(clientSocket, msg, 1200, 0);
      printf("%s",msg);
      printf("\n");
    }
    else
      printf("You have been logged out\n");
	//}while((strcmp(ch,"1")==0)||(strcmp(ch,"2")==0));
}
close(clientSocket);
}
else printf("Error connecting to server\n");
  return 0;
}
