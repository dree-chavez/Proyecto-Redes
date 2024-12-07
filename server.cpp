#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>

#define PORT 45000
#define BUFFER_SIZE 1000

int cont(int numero)
{
    // Si el número es 0, retorna 1 ya que tiene una cifra
    if (numero == 0)
    {
        return 1;
    }
    else
    {
        return std::log10(std::abs(numero)) + 1;
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
    int sock;
    socklen_t addr_len;
    ssize_t bytes_read;
    char buffer[BUFFER_SIZE + 1];
    struct sockaddr_in server_addr, client_addr;

    // Crear socket UDP
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar socket
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    addr_len = sizeof(client_addr);
    printf("UDP Server esperando clientes en el puerto %d\n", PORT);
    fflush(stdout);

    while (1)
    {
        bytes_read = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (bytes_read == -1) {
            perror("recvfrom");
            continue;
        }

        // Asegurar terminación de cadena
        if (bytes_read < BUFFER_SIZE) {
            buffer[bytes_read] = '\0';
        } else {
            buffer[BUFFER_SIZE] = '\0';
        }
        
        int tam_fname = stoi(buffer.substr(1, 5));
        string fname = buffer.substr(6, 6+tam_fname);
        int id = stoi(buffer.substr(tam_fname+6, tam_fname+10));
        int tam_data = stoi(buffer.substr(tam_fname+10, tam_fname+21));
        string data = buffer.substr(tam_fname+21,  tam_fname+21+tam_data);
        int posp=stoi(buffer.substr(984));
        int ofset=stoi(buffer.substr(985,992));
        int cks = stoi (buffer.substr(993,995));
        int pad = stoi (buffer.substr(996,999));

        string ack="a";
        ack += ceros_int(4,id);
        

        
        //printf("Mensaje recibido: %s\n", buffer);
        fflush(stdout);  

        // Enviar respuesta
        if (sendto(sock, buffer, bytes_read, 0, (struct sockaddr *)&client_addr, addr_len) == -1) {
            perror("sendto");
        }
    }

    close(sock);
    return 0;
}