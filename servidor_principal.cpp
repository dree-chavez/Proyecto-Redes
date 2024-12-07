// protocolos
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
#include <vector>
#include <sstream>
#include <iomanip> 
#include <netdb.h>

using namespace std;

// Función para calcular un checksum simple
int calculate_checksum(const std::string& data) {
    int checksum = 0;
    for (char c : data) {
        checksum += c;
    }
    return checksum % 10000; // Retorna un checksum de 4 dígitos
}

// Protocolo de envío de matriz de adyacencia
//int write_adjacency_protocol(char *buf, int total_nodes, int node, const vector<float> &data) {
// Función para escribir el protocolo de matriz de adyacencia
int write_adjacency_protocol(char* buf, int total_nodes, int node, const vector<float>& data) {
    std::ostringstream oss;
    for (size_t i = 0; i < data.size(); ++i) {
        oss << std::fixed << std::setprecision(1) << data[i];
        if (i != data.size() - 1) oss << "#";
    }
    std::string data_str = oss.str();
    int checksum = calculate_checksum(data_str);
    int data_size = data_str.size();

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "A%05d%05d%04d%06d%s", total_nodes, node, checksum, data_size, data_str.c_str());
    return 1 + 5 + 5 + 4 + 6 + data_size; // Tamaño total del mensaje
}

// Función para escribir el protocolo de matriz de características
int write_features_protocol(char* buf, int total_nodes, int node, int num_features, const std::vector<float>& data) {
    std::ostringstream oss;

    // Serializar los datos separados por '#'
    for (size_t i = 0; i < data.size(); ++i) {
        oss << std::fixed << std::setprecision(1) << data[i];
        if (i != data.size() - 1) oss << "#";
    }
    std::string data_str = oss.str();

    // Calcular checksum
    int checksum = calculate_checksum(data_str);

    // Tamaño de los datos
    int data_size = data_str.size();

    // Crear el mensaje
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "C%05d%05d%05d%04d%06d%s", total_nodes, node, num_features, checksum, data_size, data_str.c_str());

    // Imprimir el protocolo para depuración
    return 1 + 5 + 5 + 5 + 4 + 6 + data_size; // Tamaño total del mensaje
}


/////////////////////////////////////////77777
int write_sintaxis(char* buf, string& tex) {
    memset(buf, 0, sizeof(buf));
    int tam = tex.size();
    
    if (tex == "chau") {
        memset(buf, 0, sizeof(buf));
        buf[0] = 'o';
        buf[1] = '\0';
        return 1;
    } else if (tex == "Quien esta") {
        memset(buf, 0, sizeof(buf));
        buf[0] = 'l';
        buf[1] = '\0';
        return 1;
    } else {
        size_t pos = tex.find(':');
        string form = tex.substr(0, pos);

        if (form == "msg") {
            int tam_name, tam_msg;
            tex = tex.substr(pos + 1);
            size_t pos = tex.find(':');
            string name = tex.substr(0, pos);
            string msg = tex.substr(pos + 1);
            tam_name = name.size();
            tam_msg = msg.size();
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "m%04d%s%05d%s", tam_name, name.c_str(), tam_msg, msg.c_str());
            return tam_name + tam_msg + 1 + 4 + 5;
        } else if (form == "Nickname") {
            int tam_name;
            string name = tex.substr(pos + 2);
            tam_name = name.size();
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "n%04d%s", tam_name, name.c_str());
            return tam_name + 5;
        } else if (form == "MA") { // Matriz de adyacencia
            vector<float> adjacency_data = {1.5, 5, 12, 48, 13}; // Ejemplo
            int tam = write_adjacency_protocol(buf, 5, 1, adjacency_data);
            std::cout << "Protocolo matriz de adyacencia enviado: " << buf << std::endl;
            return tam;
        } else if (form == "MC") { // Matriz de características
            vector<float> feature_data = {2.3, 4.1, 3.3, 5.2}; // Ejemplo
            int tam = write_features_protocol(buf, 5, 1, 4, feature_data);
            cout << "Protocolo matriz de características enviado: " << buf << std::endl;
            return tam;
        }
        else if (form == "ACK") { // Protocolo ACK
            int packet_number = stoi(tex.substr(pos + 1)); // Obtener número de paquete
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "ACK|%05d", packet_number);
            return strlen(buf); // Retorna la longitud del mensaje calculada dinámicamente
        
        }
    }

    return 0;
}

////////////////////////////////////////////77

// Prototipos de funciones
void handle_client(int socketFD);
vector<vector<int>> deserialize_matrix(const string& data, int rows, int cols);

// Protocolo de lectura extendido para incluir las matrices
void read_thread(int socketRead) {
    int n,t;
    char buffer[100000];
    while (true) {
        n = read(socketRead, buffer, 1);  // Leer el primer carácter para identificar el protocolo
        if (n <= 0) {
            cerr << "Conexión cerrada o error de lectura." << endl;
            break;
        }
        char protocol = buffer[0];

    if (buffer[0]=='M')
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
    else if (buffer[0]== 'A') { // Protocolo matriz_adyacencia
    int total_nodes, node, tam_data;
            char checksum[5], data[1000];

            read(socketRead, buffer, 5);
            buffer[5] = '\0';
            total_nodes = atoi(buffer);

            read(socketRead, buffer, 5);
            buffer[5] = '\0';
            node = atoi(buffer);

            read(socketRead, checksum, 4);
            checksum[4] = '\0';

            read(socketRead, buffer, 6);
            buffer[6] = '\0';
            tam_data = atoi(buffer);

            read(socketRead, data, tam_data);
            data[tam_data] = '\0';

            cout << "Protocolo A recibido: Nodo " << node << " con " << tam_data << " datos.\n"
                 << "Datos: " << data << "\nChecksum: " << checksum << endl;

    }
    else if (buffer[0] == 'C') { // Protocolo matriz_características
    int total_nodes, node, num_features, tam_data;
    char checksum[5], data[10000];

    // Leer total de nodos
    read(socketRead, buffer, 5);
    buffer[5] = '\0';
    total_nodes = atoi(buffer);

    // Leer número de nodo
    read(socketRead, buffer, 5);
    buffer[5] = '\0';
    node = atoi(buffer);

    // Leer número de características
    read(socketRead, buffer, 5);
    buffer[5] = '\0';
    num_features = atoi(buffer);

    // Leer checksum
    read(socketRead, checksum, 4);
    checksum[4] = '\0';

    // Leer tamaño de los datos
    read(socketRead, buffer, 6);
    buffer[6] = '\0';
    tam_data = atoi(buffer);

    // Leer los datos
    read(socketRead, data, tam_data);
    data[tam_data] = '\0';

    cout << "Protocolo C recibido:\n";
    cout << "  Nodo: " << node << "\n";
    cout << "  Total Nodos: " << total_nodes << "\n";
    cout << "  Número de Características: " << num_features << "\n";
    cout << "  Tamaño de Datos: " << tam_data << "\n";
    cout << "  Datos: " << data << "\n";
    cout << "  Checksum: " << checksum << endl;

    // Validar checksum
    int calculated_checksum = calculate_checksum(data);
    if (calculated_checksum == atoi(checksum)) {
        cout << "Checksum válido.\n";
    } else {
        cerr << "Error: Checksum inválido. Calculado: " << calculated_checksum
             << ", Recibido: " << checksum << endl;
    }
}
    else if (protocol == 'K') { // Protocolo ACK
            char ack_buffer[10];
            n = read(socketRead, ack_buffer, 10); // Leer el contenido de ACK|NUMDEPAQUETE
            ack_buffer[n] = '\0';
            string ack_msg(ack_buffer);

            size_t pos = ack_msg.find('|');
            if (pos != string::npos && ack_msg.substr(0, pos) == "ACK") {
                string packet_num = ack_msg.substr(pos + 1);
                cout << "Se recibió el ACK de: " << packet_num << endl;
            } else {
                cerr << "Formato de ACK inválido: " << ack_msg << endl;
            }

    }

    else {
            cerr << "Protocolo desconocido: " << protocol << endl;
        }
    }
}

// Función para deserializar matrices
vector<vector<int>> deserialize_matrix(const string& data, int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    istringstream stream(data);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            stream >> matrix[i][j];
        }
    }
    return matrix;
}

// Servidor principal
int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;
    struct hostent *host;
    int Res;

    // Resolver nombre de host
    host = gethostbyname("18.117.146.57");
    if (host == nullptr) {
        perror("Error al resolver el nombre del host");
        exit(EXIT_FAILURE);
    }

    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1) {
        perror("No se puede crear el socket");
        exit(EXIT_FAILURE);
    }


   // Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(45000);
    serverAddr.sin_addr = *((struct in_addr *)host->h_addr);

    // Conectar al servidor
    Res = connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in));
    if (Res == -1) {
        perror("Fallo al conectar");
        close(serverSocket);
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
    n = write(serverSocket,s,tam);
    char buffer[255];
    thread(read_thread,serverSocket).detach();
    do{
        getline(cin, buff);
        int tam = write_sintaxis(s,buff);
        n = write(serverSocket,s,tam);

        //vector<float> adjacency_data = {1.5, 5, 12, 48, 13};
        //int tam1 = write_adjacency_protocol(buffer, 5, 1, adjacency_data);
        //write(serverSocket, buffer, tam1);
    }
    while(buff!="chau");
    
 
  shutdown(serverSocket, SHUT_RDWR);
    return 0;
}
