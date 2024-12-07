// server  final tcp 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>

constexpr int PORT = 1100;
#define BUFFER_SIZE 1000
using namespace std;

int cont(int numero)
{
    // Si el número es 0, retorna 1 ya que tiene una cifra
    if (numero == 0)
    {
        return 1;
    }
    else
    {
        return log10(abs(numero)) + 1;
    }
}

string ceros_int(int tam, int num)
{
    string ret;
    if (cont(num) < tam)
    {
        int total = tam - cont(num);
        for (int i = 0; i < total; i++)
        {
            ret = ret + "0";
        }
    }
    return ret + to_string(num);
}

int main()
{
     int serverSocket;
    struct sockaddr_in server_addr,  client_addr;
    socklen_t addr_len;
    ssize_t bytes_read;
    char buffer[BUFFER_SIZE + 1];
    // Crear socket UDP
    if ((serverSocket= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar socket
    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(serverSocket, 5) == -1) {
        perror("Listen");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    
    
    printf("TCP Server esperando clientes en el puerto %d\n", PORT);
    fflush(stdout);

    addr_len = sizeof(client_addr);

    for(;;)
    {
        int clientSocket = accept(serverSocket, NULL, NULL);
      
        if (0> clientSocket)
        {
            perror("error accept failed");
            close(clientSocket);
            exit(EXIT_FAILURE);
        }
        if (bytes_read == -1) {
            perror("recv");
            close(clientSocket);
            continue;
        }

        // Asegurar terminación de cadena
        if (bytes_read < BUFFER_SIZE) {
            buffer[bytes_read] = '\0';
        } else {
            buffer[BUFFER_SIZE] = '\0';
        }
        
        // Procesar datos recibidos
        int tam_fname = stoi(string(buffer).substr(1, 5));
        string fname = string(buffer).substr(6, 6+tam_fname);
        int id = stoi(string(buffer).substr(tam_fname+6, tam_fname+10));
        int tam_data = stoi(string(buffer).substr(tam_fname+10, tam_fname+21));
        string data = string(buffer).substr(tam_fname+21,  tam_fname+21+tam_data);
        int posp=stoi(string(buffer).substr(984));
        int ofset=stoi(string(buffer).substr(985,992));
        int cks = stoi (string(buffer).substr(993,995));
        int pad = stoi (string(buffer).substr(996,999));

        string ack="a";
        ack += ceros_int(4,id);
        

        
        //printf("Mensaje recibido: %s\n", buffer);
        fflush(stdout);  

        // Enviar respuesta
        if (send(clientSocket, buffer, bytes_read, 0) == -1) {
            perror("send");
        }
      
        
    }
 
    // Cerrar el socket del servidor
    close(serverSocket);
    return 0;
}
