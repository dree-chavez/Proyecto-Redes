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
#include <map>
#include <vector>
#include <list>
#include <unordered_map>
#include <set>
#include <mutex>
#include <sstream>

using namespace std;

// Estructura para representar un grafo
struct Graph {
    int numNodes;                         // Número de nodos
    vector<vector<float>> adjacency;      // Matriz de adyacencia (cambiado a float)
    vector<vector<int>> features;         // Matriz de características
};
// Grafo compartido
Graph graph;
// Mutex para proteger el acceso al grafo
std::mutex graphMutex;
vector<float> parse_data(const string& data) {
    vector<float> parsed_data;
    stringstream ss(data); // Crear flujo para los datos
    string value;
    while (getline(ss, value, '#')) { // Leer valores separados por '#'
        parsed_data.push_back(stof(value)); // Convertir a float y agregar al vector
    }
    return parsed_data;
}

void update_adjacency_matrix(Graph& graph, int node, const vector<float>& adjacency_data) {
    if (node >= graph.numNodes) {
        cerr << "Error: Nodo fuera de rango." << endl;
        return;
    }

    // Si la fila no tiene datos, inicialízala
    if (graph.adjacency.size() <= node) {
        graph.adjacency.resize(node + 1);
    }

    // Actualiza la fila correspondiente con los datos de adyacencia
    graph.adjacency[node] = adjacency_data;

    cout << "Nodo " << node << " actualizado en la matriz de adyacencia." << endl;
}


int calculate_checksum(const string& data) {
    int checksum = 0;
    for (char c : data) {
        checksum += c;
    }
    return checksum % 10000; // Retorna un checksum de 4 dígitos
}
// Mapas y listas para clientes y esclavos
list<int> listaC;
map<string, int> ClientMap;
list<int> listaE;
map<string, int> EsclavosMap;

// Mutex para proteger los mapas compartidos
std::mutex clientMapMutex;
std::mutex esclavosMapMutex;

// Función para registrar clientes
void register_map(int socket, string name) {
    std::lock_guard<std::mutex> lock(clientMapMutex);
    ClientMap[name] = socket;
}

// Función para registrar esclavos
void register_map_esclavos(int socket, string name) {
    std::lock_guard<std::mutex> lock(esclavosMapMutex);
    EsclavosMap[name] = socket;
}

// Función para encontrar un socket por nombre en ClientMap
int encontrarSocketPorNombre(map<string, int>& clientMap, const string& name) {
    auto it = clientMap.find(name);
    if (it != clientMap.end()) {
        return it->second;
    }
    return -1;
}

// Función para encontrar un nombre por socket en ClientMap
string encontrarNombrePorSocket(map<string, int>& clientMap, int socket) {
    for (auto it = clientMap.begin(); it != clientMap.end(); ++it) {
        if (it->second == socket) {
            return it->first;
        }
    }
    return "";
}

// Función para encontrar un socket por nombre en EsclavosMap
int encontrarSocketPorNombreE(map<string, int>& EsclavosMap, const string& name) {
    auto it = EsclavosMap.find(name);
    if (it != EsclavosMap.end()) {
        return it->second;
    }
    return -1;
}

// Función para encontrar un nombre por socket en EsclavosMap
string encontrarNombrePorSocketE(map<string, int>& EsclavosMap, int socket) {
    for (auto it = EsclavosMap.begin(); it != EsclavosMap.end(); ++it) {
        if (it->second == socket) {
            return it->first;
        }
    }
    return "";
}

// Función para dividir un grafo en subgrafos
unordered_map<int, set<int>> divideGraph(const Graph& graph, int numSlaves) {
    unordered_map<int, set<int>> subgraphs;
    if (numSlaves == 0) {
        cerr << "Error: número de esclavos no puede ser cero." << endl;
        return subgraphs;
    }

    int nodesPerSlave = graph.numNodes / numSlaves;
    int remainingNodes = graph.numNodes % numSlaves;
    int currentNode = 0;

    for (int slave = 0; slave < numSlaves; ++slave) {
        int nodesToAssign = nodesPerSlave + (remainingNodes > 0 ? 1 : 0);
        if (remainingNodes > 0) remainingNodes--;

        for (int i = 0; i < nodesToAssign; ++i) {
            subgraphs[slave].insert(currentNode);
            currentNode++;
        }
    }

    return subgraphs;
}
// Hilo para manejar lectura de clientes
void read_thread(int socketRead) {
    int n, t;
    char buffer[1000];

    while (true) {
        char s[1000];
        n = read(socketRead, buffer, 1);
        if (n <= 0) break; // Manejo básico de desconexión

        if (buffer[0] == 'b') {
            string name;
            n = read(socketRead, buffer, 5);
            buffer[n] = '\0';
            t = atoi(buffer);
            n = read(socketRead, buffer, t);
            buffer[n] = '\0';
            name = encontrarNombrePorSocket(ClientMap, socketRead);
            int tam_name = name.size();
            string msg = string(buffer);
            int tam_msg = msg.size();

            if (tam_msg == 0) {
                string error = "Msg vacio";
                int tam_error = error.size();
                sprintf(s, "E%05d%s", tam_error, error.c_str());
                write(socketRead, s, 6 + tam_error);
            } else {
                sprintf(s, "B%04d%s%05d%s", tam_name, name.c_str(), tam_msg, msg.c_str());
                for (auto& client : ClientMap) {
                    write(client.second, s, 10 + tam_name + tam_msg);
                }
            }
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
                 // Validar checksum
            int calculated_checksum = calculate_checksum(data);
            if (calculated_checksum != atoi(checksum)) {
                cerr << "Error: Checksum inválido. Calculado: " << calculated_checksum
                     << ", Recibido: " << checksum << endl;
                continue;
            }
            // Procesar los datos recibidos
    vector<float> adjacency_data = parse_data(data);
    update_adjacency_matrix(graph, node, adjacency_data);

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
        // Otros protocolos pueden ser manejados aquí...
    }
    close(socketRead); // Cerrar el socket al finalizar
}

// Función principal
int main() {
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (-1 == SocketFD) {
        perror("No se puede crear el socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("Error al asociar el socket");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == listen(SocketFD, 10)) {
        perror("Error al escuchar");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        int ClientFD = accept(SocketFD, NULL, NULL);
        if (0 > ClientFD) {
            perror("Error al aceptar la conexión");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }

        char buffer[1000];
        int n = read(ClientFD, buffer, 1);
        if (n <= 0) continue;

        if (buffer[0] == 'n') {
            string name;
            n = read(ClientFD, buffer, 4);
            buffer[n] = '\0';
            int t = atoi(buffer);
            read(ClientFD, buffer, t);
            name = string(buffer);
            register_map(ClientFD, name);
            printf("Se conectó: [%s]\n", name.c_str());
        } else if (buffer[0] == 'E') {
            string name;
            n = read(ClientFD, buffer, 4);
            buffer[n] = '\0';
            int t = atoi(buffer);
            read(ClientFD, buffer, t);
            name = string(buffer);
            register_map_esclavos(ClientFD, name);
            printf("Se conectó esclavo: [%s]\n", name.c_str());
        }

        thread(read_thread, ClientFD).detach();
    }

    close(SocketFD);
    return 0;
}
