/****************** SERVER CODE ****************/
/****************** SERVER CODE ****************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>
#include <unistd.h> //write
#include <pthread.h> //for threading , link with lpthread
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

struct rec{
  int valid;
  char sername[1024];
  char port[5];
  char userfile[1024];
};

char dname[20],flcat[50],fname[100];

struct rec checkwithCS(char buffer[1024]){
  struct rec r;
  int clientSocket;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(9000);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  addr_size = sizeof serverAddr;
  connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
  printf("Connected to Central server\n");
  send(clientSocket, "check", 1024, 0);
  send(clientSocket, buffer, 1024, 0);
  recv(clientSocket, &r, sizeof(r), 0);
  printf("Data received\n");
      printf("%d\n%s\n%s\n%s\n",r.valid,r.sername,r.port,r.userfile);
  return r;
}

void initialize(char sername[1024],char port[5],char userfile[1024]){
  struct rec r;
  char buffer[1024];
      r.valid=1;
      strcpy(r.sername,sername);
      strcpy(r.port,port);
      strcpy(r.userfile,userfile);
  int clientSocket;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(9000);
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  addr_size = sizeof serverAddr;
  connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
  printf("Connected to Central server\n");
  send(clientSocket, "init", 1024, 0);
  send(clientSocket, &r, sizeof(r), 0);
  recv(clientSocket, buffer, sizeof(r), 0);
  printf("%s\n",buffer);
      //printf("%d\n%s\n%s\n%s\n",r.valid,r.sername,r.port,r.userfile);
}

void *connection_handler(void *);

int main(int argc, char **argv){
  int welcomeSocket, newSocket;
  char run[1024],rpass[1024];
  char buffer[1024],fm[1200],bu[1024];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  FILE *fp;

  strcpy(dname,"gmail");
  strcpy(flcat,"");
  strcpy(fname,"users");
  strcat(flcat,"@");
  strcat(flcat,dname);
  strcat(flcat,".txt");
  strcat(fname,flcat);
 // puts(dname);
 // puts(fname);
  initialize("gmail.com","9998",fname);

  char *hostname = argv[1];     
  char ip[100];     
  get_ip(hostname , ip);     
  printf("%s resolved to %s\n" , hostname , ip);

  int pid=fork();
  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  welcomeSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (welcomeSocket == -1)
    {
    printf("Could not create socket");
    }

  serverAddr.sin_family = AF_INET;

  if(pid!=0) serverAddr.sin_port = htons(9998);
  else serverAddr.sin_port = htons(atoi(argv[2]));

  if(pid!=0) serverAddr.sin_addr.s_addr = INADDR_ANY;
  else serverAddr.sin_addr.s_addr = inet_addr(ip);

  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    if( bind(welcomeSocket,(struct sockaddr *)&serverAddr , sizeof(serverAddr)) < 0)
    {
    perror("bind failed. Error");
    return 1;
    }

  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  puts("Waiting for incoming connections...");


  if(pid!=0){
  for ( ; ; ) {
   printf("Waiting...\n");
   addr_size = sizeof serverStorage;
   newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
   printf("\n%s\n","Received request...");

   int childpid;
   if ( (childpid = fork ()) == 0 ) {//if it’s 0, it’s child process
    close (welcomeSocket);
    recv(newSocket, run, 1024, 0);
    recv(newSocket, fm, 1200, 0);
    printf("Message and receiver's username received\n");   
    
     fp=fopen(strcat(run,flcat),"a");
     fputs(fm,fp);
     strcpy(buffer,"Valid recipient, mail sent");
     fclose(fp);
    
    printf("%s\n",buffer);
    send(newSocket,buffer,1024,0);
    exit(0);
  }  
  printf("Closing socket...\n");
  close(newSocket);
  }
}


else{
  addr_size = sizeof serverStorage;
  pthread_t thread_id;
  while( (newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size)) )
  {
   puts("\nConnection accepted");
   if( pthread_create( &thread_id , NULL , connection_handler , (void*) &newSocket) < 0)
    {
    perror("could not create thread");
    return 1;
    }
  }
  if (newSocket < 0)
  {
   perror("accept failed");
   return 1;
  }
  close(newSocket);
}
  }
  

void *connection_handler(void *socket_desc)
{
  FILE *fp;
  int i,j,newSocket = *(int*)socket_desc;
  char ch[5],buffer[1024],sun[50],ser[50],run[50],un[50],bu[1024],spass[20],fm[1200],buffer1[1024];
  recv(newSocket, sun, 50, 0);
  recv(newSocket, spass, 20, 0);
  printf("Username and password received %s\n",fname);

  fp=fopen(fname,"a");
  fclose(fp);
  fp=fopen(fname,"r");
  int flag=0;
  while(!feof(fp) /*&& flag!=1*/)
  {
    fscanf(fp,"%s",buffer);
    fscanf(fp,"%s",buffer1);
    //fgets(buffer,1024,fp);
    //fgets(buffer1,1024,fp);
    if(strcmp(buffer,sun)==0) {
      flag=1;
      if(strcmp(buffer1,spass)==0)
        strcpy(buffer,"Login successful");
      else strcpy(buffer,"Wrong password");
      break;
    }
  }
  fclose(fp);

  if(flag==0)
  {
    fp=fopen(fname,"a");
    fprintf(fp,"%s\n%s\n",sun,spass);
    strcpy(buffer,"User registered");
    fclose(fp);
  }
  send(newSocket,buffer,1024,0);
  printf("%s\n",buffer);

  if(strcmp(buffer,"Login successful")==0 || strcmp(buffer,"User registered")==0){
    //while(
      recv(newSocket,ch,5,0); //){
      printf("Option %s chosen\n",ch);
      if(strcmp(ch,"1")==0){
                  recv(newSocket, un, 1024,0);
                  recv(newSocket, fm, 1200,0);
                  printf("Message and receiver's username received\n %s \n %s \n",un,fm);   

                  for(i=0;un[i]!='\0';i++){
                    if(un[i]=='@'){
                      run[i]='\0';
                      for(i++;un[i]!='\0';i++)
                      ser[j++]=un[i];
                      break;
                    }
                    else run[i]=un[i];
                  }
                  run[i]='\0';
                  puts(run);
                  puts(ser);
                  struct rec r=checkwithCS(ser);

      //printf("%d\n%s\n%s\n%s\n",r.valid,r.sername,r.port,r.userfile);
                  if(r.valid==1){
                      printf("Opening %s...\n",r.userfile);
                      fp=fopen(r.userfile,"a");
                      fclose(fp);
                      fp=fopen(r.userfile,"r");
                      int flag1=0;
                      while(!feof(fp) /*&&flag1!=1*/)
                      {
                      fscanf(fp,"%s",buffer);
                      fscanf(fp,"%s",bu);
                      if(strcmp(buffer,run)==0)
                      {flag1=1;
                       break;}
                      }
                      fclose(fp);
                      printf("Search done\n");

                      if(flag1!=1)
                       send(newSocket,"Invalid recipient",50,0);
                      else { 
                      
                          int clientSocket;
                          struct sockaddr_in serverAddr;
                          socklen_t addr_size;
                          clientSocket = socket(PF_INET, SOCK_STREAM, 0);
                          serverAddr.sin_family = AF_INET;
                          serverAddr.sin_port = htons(atoi(r.port));
                          serverAddr.sin_addr.s_addr = INADDR_ANY;
                          memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  
                          addr_size = sizeof serverAddr;

                          int flag2=1,flag3=0;
                          if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)==-1){
                            flag2=0;
                            send(newSocket,"Mail is being sent!",50,0);
                            int p=fork();
                            if(p==0){
                            printf("Couldn't connect to receiver side\n");
                            while(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)==-1);
                            flag2=1; flag3=1;
                            }
                          }
                          if(flag2==1){
                              printf("Connected to receiver side\n");
                              send(clientSocket,run,1024,0);
                              send(clientSocket,fm,1200,0); 
                              recv(clientSocket,buffer,50,0);
                              printf("%s\n",buffer);                              
                              if(flag3!=1) send(newSocket,"Mail sent!",50,0); 
                              close(clientSocket);
                          }
                    }
              }
                  else {
                    printf("Receiver invalid\n");
                    send(newSocket,"Receiver invalid!",50,0);}
      }

      else if(strcmp(ch,"2")==0){
        strcpy(run,sun);
        fp=fopen(strcat(run,flcat),"r");
        printf("Loading inbox...\n");
        if(fp==NULL) {strcpy(buffer,"Inbox empty!"); send(newSocket,buffer,1200,0);}
        else{
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);  //same as rewind(f);

        char *string = malloc(fsize + 1);
        fread(string, fsize, 1, fp);
        send(newSocket,string,1200,0);
        fclose(fp);
        }
      }

      // else break;
    //} 
  }

  printf("Closing socket...\n");
}






