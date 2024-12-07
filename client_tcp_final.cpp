// client
#include <iostream>
#include <iomanip> // Encabezado necesario para setfill y setw
#include <sstream> // Encabezado para ostringstream
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <map>
#include <cmath> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <vector>
using namespace std;
using namespace std::chrono;
constexpr int PORT = 1100;
constexpr char SERVER_ADDRESS[] = "127.0.0.1"; // Cambia esta dirección IP si el servidor está en otra máquina


map<int, chrono::system_clock::time_point> mimapa;
mutex mtx;
ofstream logFile; // Archivo para guardar tiempos

int sock;
struct sockaddr_in server_addr;
unsigned int addr_len;
int pipelined = 0;
int ix = 0;

string calcularChecksum(const vector<char> &mensaje) {
    int checksum = 0;
    for (unsigned char c : mensaje) checksum += c;
    std::ostringstream checksumStream;
    checksumStream << setw(3) << setfill('0') << (checksum % 256);
    cout << checksumStream.str() << ": checksum" << endl;
    return checksumStream.str();
}

string crearFragmento(const vector<char> &mensaje, int numFragmentosTotales, int numFragmentoActual) {
    if (mensaje.size() > 985) throw invalid_argument("El mensaje no puede exceder los 985 bytes.");

    string fragmento(mensaje.begin(), mensaje.end());
    int rellenoSize = 1000 - mensaje.size() - 15;
    fragmento += string(rellenoSize, '#');
    char flag = (numFragmentosTotales == 1 || numFragmentoActual == numFragmentosTotales - 1) ? '0' : '1';
    fragmento += flag;
    int offset = numFragmentoActual > 0 ? numFragmentoActual * 985 : 0;
    string offsetStr = string(7 - to_string(offset).length(), '0') + to_string(offset);
    fragmento += offsetStr;
    fragmento += calcularChecksum(mensaje);
    string rellenoSizeStr = string(4 - to_string(rellenoSize).length(), '0') + to_string(rellenoSize);
    fragmento += rellenoSizeStr;

    return fragmento;
}

void readTCP() {
    char buffer[1001];
    while (true) {
        int bytes_read = recv(sock, buffer, 1000, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            int offset;
            sscanf(buffer, "ACK %d", &offset);
            int id = offset;

            system_clock::time_point tp2 = system_clock::now();
            std::lock_guard<std::mutex> lock(mtx);

            if (strncmp(buffer, "ACK", 3) == 0) {
                if (mimapa.find(id) != mimapa.end()) {
                    auto tp1 = mimapa[id];
                    auto diff = duration_cast<milliseconds>(tp2 - tp1);

                    // Registrar datos en archivo
                    logFile << "Offset: " << id << ", Tiempo: " << diff.count() << " ms" << endl;

                    // Mostrar en consola
                    cout << "ID " << id << " Tiempo ida y vuelta: " << diff.count() << " ms" << endl;
                    cout << "ACK del fragmento -> " << offset << endl;

                    mimapa.erase(offset);
                    pipelined--;
                }
            } else {
                cout << "NACK recibido para el offset " << offset << endl;
                pipelined--;
            }
        }
    }
}


int main() {
    // Crear socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Error creando socket\n";
        return 1;
    }
    struct hostent *host;
    char send_data[1024];

    //host = (struct hostent *)gethostbyname((char *)"18.117.146.57");
    host = (struct hostent *)gethostbyname((char *)"127.0.0.1");

    
    // Configurar la dirección del servidor
    struct sockaddr_in server_addr{};
    
    server_addr.sin_family = AF_INET;
    //server_addr.sin_port = htons(45000)
    //server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    server_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0) {
        cerr << "Dirección IP no válida\n";
        return 1;
    }
    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Error conectando al servidor\n";
        return 1;
    }
    cout << "Conectado al servidor en " << SERVER_ADDRESS << ":" << PORT << endl;

    // Abrir archivo para registro
    logFile.open("tiempos.csv", ios::out | ios::app);
    if (!logFile) {
        cerr << "Error al abrir el archivo de log." << endl;
        return 1;
    }
    logFile << "Offset,Tiempo (ms)" << endl; // Encabezado del archivo CSV

    pipelined = 0;
    thread(readTCP).detach();

    logFile.close(); // Cerrar archivo al final
    close(sock);
    return 0;
}