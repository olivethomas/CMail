/****************** SERVER CODE ****************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include<unistd.h> //write
#include<pthread.h> //for threading , link with lpthread
//the thread function
void *connection_handler(void *);

struct rec{
  int valid;
  char sername[1024];
  char port[5];
  char userfile[1024];
};

int main(int argc, char **argv){
  int welcomeSocket, newSocket;
  char inp[1024],b[10];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  welcomeSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (welcomeSocket == -1)
    {
    printf("Could not create socket");
    }

  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(9000);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Bind the address struct to the socket ----*/
    if( bind(welcomeSocket,(struct sockaddr *)&serverAddr , sizeof(serverAddr)) < 0)
    {
    //print the error message
    perror("bind failed. Error");
    return 1;
    }

  /*---- Listen on the socket, with 5 max connection requests queued ----*/
  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  puts("Waiting for incoming connections...");
  addr_size = sizeof serverStorage;
  pthread_t thread_id;
  while( (newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size)) )
  {
   puts("Connection accepted");
   if( pthread_create( &thread_id , NULL , connection_handler , (void*) &newSocket) < 0)
    {
    perror("could not create thread");
    return 1;
    }
    //Now the thread , so that we dont terminate before the thread
    //pthread_join( thread_id , NULL);
    puts("Handler assigned");
  }
  if (newSocket < 0)
  {
   perror("accept failed");
   return 1;
  }
  close(newSocket);
  return 0;
}
  

void *connection_handler(void *socket_desc)
{
  int newSocket = *(int*)socket_desc;
  char ser[1024],buffer1[1024],buffer2[1024];
  char buffer[1024];
  FILE *fp;
  struct rec r;
recv(newSocket, buffer, 1024, 0);
if(strcmp(buffer,"check")==0){

  recv(newSocket, ser, 1024, 0);
  fp=fopen("receivers.txt","r");
  int flag=0;
  while(!feof(fp) /*&& flag!=1*/)
  {
    fscanf(fp,"%s",buffer);
    fscanf(fp,"%s",buffer1);
    fscanf(fp,"%s",buffer2);
    //fgets(buffer,1024,fp);
    //fgets(buffer1,1024,fp);
    if(strcmp(buffer,ser)==0) {
      flag=1;
      printf("Receiver found\n");
      r.valid=1;
      strcpy(r.sername,buffer);
      strcpy(r.port,buffer1);
      strcpy(r.userfile,buffer2);
      printf("%d\n%s\n%s\n%s\n",r.valid,r.sername,r.port,r.userfile);
      break;
    }
  }
  fclose(fp);

  if(flag==0){
      r.valid=0;
      strcpy(buffer,"");
      strcpy(buffer1,"");
      strcpy(buffer2,"");
      printf("%d\n%s\n%s\n%s\n",r.valid,r.sername,r.port,r.userfile);
  }
  printf("Sending data\n");
  send(newSocket,&r,sizeof(r),0);
  printf("Closing socket\n");
}
else if(strcmp(buffer,"init")==0){
  recv(newSocket, &r, sizeof(r), 0);
  fp=fopen("receivers.txt","r");
  int flag=0;
  while(!feof(fp) /*&& flag!=1*/)
  {
    fscanf(fp,"%s",buffer);
    fscanf(fp,"%s",buffer1);
    fscanf(fp,"%s",buffer2);
    //fgets(buffer,1024,fp);
    //fgets(buffer1,1024,fp);
    if(strcmp(buffer,r.sername)==0) {
      flag=1;
      strcpy(buffer,"Receiver already initialized\n");
      break;
    }
  }
  if(flag==0){
      fp=fopen("receivers.txt","a");
    fprintf(fp,"%s\n%s\n%s\n",r.sername,r.port,r.userfile);
    strcpy(buffer,"Server registered");
    fclose(fp);
  }
  send(newSocket,buffer,1024,0);
  printf("%s\n",buffer);
  fclose(fp);

}
}
