 /* Client code in C */
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <netdb.h>


using namespace std;

int write_sintaxis(char * buf, string &tex)
{
    memset(buf, 0, sizeof(buf));
    int tam = tex.size();
    if(tex=="chau")
    {
        memset(buf, 0, sizeof(buf));
        buf[0]='o';
        buf[1]='\0';
        return 1;
    }
    else if(tex=="Quien esta"){
        memset(buf, 0, sizeof(buf));
        buf[0]='l';
        buf[1]='\0';
        return 1;
    }
    else
    {
        size_t pos = tex.find(':');
        string form= tex.substr(0, pos);
        if (form=="msg")
        {
            int tam_name,tam_msg;
            tex=tex.substr(pos + 1);
            size_t pos = tex.find(':');
            string name = tex.substr(0, pos);
            string msg = tex.substr(pos + 1);
            tam_name=name.size();
            tam_msg=msg.size();
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "m%04d%s%05d%s", tam_name, name.c_str(),tam_msg,msg.c_str() );
            //printf("[%s]\n",buf);
            return tam_name+tam_msg+1+4+5;
        }
        else if(form=="broadkast")
        {
            int tam_msg;
            string msg = tex.substr(pos + 1);
            tam_msg=msg.size();
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "b%05d%s", tam_msg, msg.c_str());
            //printf("[%s]\n",buf);
            return tam_msg+6;
        }
        
        else if(form=="Nickname"){
            int tam_name;
            string name = tex.substr(pos + 2);
            tam_name=name.size();
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "n%04d%s", tam_name, name.c_str());
            //printf("[%s]\n",buf);
            return tam+5;
        }
        else if(form=="JF"){
            int tam_fname,tam_name;
            string fname , nname;
            tex=tex.substr(pos + 1);
            pos = tex.find(':');
            fname = tex.substr(0,pos);
            nname = tex.substr(pos+1);
            tam_name = nname.size();
            tam_fname=fname.size();
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "F%05d%s%05d%s", tam_fname, fname.c_str(),tam_name,nname.c_str());
            //printf("[%s]\n",buf);
            return tam+5;
        }
    }
    return 0;
}


void read_thread(int socketRead)
{
  int n,t;
  char buffer[100000];
  while(1){
    n = read(socketRead,buffer,1);
    if (buffer[0]=='B')
    {
      string name,msg;
      n = read(socketRead,buffer,4);
      buffer[n]='\0';
      t = atoi(buffer);
      n = read(socketRead,buffer,t);
      buffer[n]='\0';
      name = string(buffer); 
      memset(buffer, 0, sizeof(buffer));
      n = read(socketRead,buffer,5);
      buffer[n]='\0';
      t = atoi(buffer);
      n = read(socketRead,buffer,t);
      buffer[n]='\0';
      msg = string(buffer); 
      cout<<name<<": "<<msg<<endl;
      memset(buffer, 0, sizeof(buffer));
      memset(&msg, 0, sizeof(msg));
      memset(&name, 0, sizeof(name));
    }
    else if (buffer[0]=='M')
    {
      int SocketFinal;
      int tam_name,tam_msg;
      n = read(socketRead,buffer,4);
      buffer[n]='\0';
      t = atoi(buffer);
      n = read(socketRead,buffer,t);
      string name = string(buffer);
      memset(buffer, 0, sizeof(buffer));
      n = read(socketRead,buffer,5);
      buffer[n]='\0';
      t = atoi(buffer);
      n = read(socketRead,buffer,t);
      string msg = string(buffer); 
      cout<<name<<": "<<msg<<endl;
      memset(buffer, 0, sizeof(buffer));
      memset(&msg, 0, sizeof(msg));
      memset(&name, 0, sizeof(name));
    }
    else if (buffer[0]=='L')
    {
      n = read(socketRead,buffer,5);
      buffer[n]='\0';
      t = atoi(buffer);
      n = read(socketRead,buffer,t);
      string list = string(buffer);
      cout<<"Conectados: "<<list<<endl;
      memset(buffer, 0, sizeof(buffer));
    }
    else if (buffer[0]=='E')
    {
      n = read(socketRead,buffer,5);
      buffer[n]='\0';
      t = atoi(buffer);
      n = read(socketRead,buffer,t);
      string error = string(buffer);
      cout<<"ERROR: "<<error<<endl;
      memset(buffer, 0, sizeof(buffer));
      memset(&error, 0, sizeof(error));
    }

  }
} 
int main(void)
{
  int SocketFD;
    struct sockaddr_in stSockAddr;
    struct hostent *host;
    int Res;

    // Resolver nombre de host
    host = gethostbyname("18.117.146.57");
    if (host == nullptr) {
        perror("Error al resolver el nombre del host");
        exit(EXIT_FAILURE);
    }

    // Crear socket TCP
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1) {
        perror("No se puede crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    stSockAddr.sin_addr = *((struct in_addr *)host->h_addr);

    // Conectar al servidor
    Res = connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
    if (Res == -1) {
        perror("Fallo al conectar");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    std::cout << "Conectado al servidor TCP exitosamente.\n";

	int n;  
  string buff;
  string nicname="Nickname: ";
  cout << nicname;
  getline(cin, buff);
  char s[100000];
  nicname=nicname+buff;
  int tam = write_sintaxis(s,nicname);
  n = write(SocketFD,s,tam);
  char buffer[255];
  thread(read_thread,SocketFD).detach();
  do{
    getline(cin, buff);
    int tam = write_sintaxis(s,buff);
    n = write(SocketFD,s,tam);
  }while(buff!="chau");
    
  /* perform read write operations ... */
 
  shutdown(SocketFD, SHUT_RDWR);

  return 0;
}
