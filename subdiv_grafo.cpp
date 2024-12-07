#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>

// Estructura para representar un grafo
struct Graph {
    int numNodes;                            // Número de nodos
    std::vector<std::vector<int>> adjacency; // Matriz de adyacencia
    std::vector<std::vector<int>> features;  // Matriz de características
};

// Función para dividir un grafo en subgrafos
std::unordered_map<int, std::set<int>> divideGraph(const Graph& graph, int numSlaves) {
    std::unordered_map<int, std::set<int>> subgraphs; // Subgrafos resultantes
    int nodesPerSlave = graph.numNodes / numSlaves;   // Nodos por esclavo
    int remainingNodes = graph.numNodes % numSlaves;  // Nodos sobrantes

    int currentNode = 0;

    for (int slave = 0; slave < numSlaves; ++slave) {
        int nodesToAssign = nodesPerSlave + (remainingNodes > 0 ? 1 : 0); // Asignar nodos extra si hay
        if (remainingNodes > 0) remainingNodes--; // Reducir nodos sobrantes

        for (int i = 0; i < nodesToAssign; ++i) {
            subgraphs[slave].insert(currentNode);
            currentNode++;
        }
    }

    return subgraphs;
}

// Función para imprimir los subgrafos
void printSubgraphs(const std::unordered_map<int, std::set<int>>& subgraphs) {
    for (const auto& [slave, nodes] : subgraphs) {
        std::cout << "Subgrafo para esclavo " << slave << ": ";
        for (int node : nodes) {
            std::cout << node << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    // Ejemplo de entrada: grafo con 10 nodos y 4 esclavos
    Graph graph;
    graph.numNodes = 10;
    graph.adjacency = {
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 1, 1}
    };
    graph.features = {
        {1, 0, 2, 1, 0, 3, 1, 0, 1},
        {0, 1, 0, 2, 3, 1, 0, 1, 0},
        {1, 1, 1, 0, 0, 2, 1, 0, 1},
        {2, 0, 1, 1, 1, 0, 0, 1, 2},
        {0, 0, 1, 1, 3, 1, 0, 2, 1},
        {1, 1, 0, 0, 2, 1, 1, 0, 1},
        {2, 0, 1, 1, 1, 0, 2, 1, 0},
        {0, 1, 1, 0, 0, 1, 1, 0, 2},
        {1, 0, 2, 1, 0, 3, 1, 0, 1},
        {0, 2, 0, 1, 3, 1, 0, 1, 0}
    };

    int numSlaves = 4;
    auto subgraphs = divideGraph(graph, numSlaves);

    printSubgraphs(subgraphs);

    return 0;
}
