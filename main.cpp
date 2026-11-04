#include "Graph.hpp"

#include <iostream>
#include <string>

// -----------------------------------------------------------------------
// Programa de exemplo / teste da biblioteca de grafos.
//
// Uso:
//   ./graph_program <arquivo_de_entrada> <matrix|list> [vertice_inicial_busca]
//
// Gera:
//   output_info.txt   -> numero de vertices/arestas, graus, componentes
//   output_bfs.txt    -> arvore de busca em largura (pai e nivel)
//   output_dfs.txt    -> arvore de busca em profundidade (pai e nivel)
//   output_dijkstra.txt -> distancia e pai (Dijkstra) a partir de vertice_inicial
// -----------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0]
                  << " <arquivo_de_entrada> <matrix|list> [vertice_inicial]\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string repArg = argv[2];
    int startVertex = (argc >= 4) ? std::stoi(argv[3]) : 1;

    Graph::Representation rep;
    if (repArg == "matrix") {
        rep = Graph::Representation::MATRIX;
    } else if (repArg == "list") {
        rep = Graph::Representation::LIST;
    } else {
        std::cerr << "Representacao invalida. Use 'matrix' ou 'list'.\n";
        return 1;
    }

    try {
        Graph g(rep);
        g.loadFromFile(inputFile);

        std::cout << "Grafo carregado: " << g.getNumVertices() << " vertices, "
                  << g.getNumEdges() << " arestas.\n";

        g.writeGraphInfo("output_info.txt");
        std::cout << "Informacoes gerais escritas em output_info.txt\n";

        g.bfs(startVertex, "output_bfs.txt");
        std::cout << "Arvore de BFS (a partir do vertice " << startVertex
                  << ") escrita em output_bfs.txt\n";

        g.dfs(startVertex, "output_dfs.txt");
        std::cout << "Arvore de DFS (a partir do vertice " << startVertex
                  << ") escrita em output_dfs.txt\n";

        int diamExact = g.diameter();
        int diamApprox = g.diameterApprox();
        std::cout << "Diametro exato: " << diamExact << "\n";
        std::cout << "Diametro aproximado (double sweep): " << diamApprox << "\n";

        // Parte 2: Dijkstra (so se o grafo tiver pesos nao-negativos).
        if (g.hasNegativeWeights()) {
            std::cout << "Grafo com pesos negativos: a biblioteca ainda nao "
                         "implementa caminhos minimos nesse caso.\n";
        } else {
            g.dijkstra(startVertex, "output_dijkstra.txt", Graph::DijkstraMode::HEAP);
            std::cout << "Caminhos minimos (Dijkstra com heap, a partir de "
                      << startVertex << ") escritos em output_dijkstra.txt\n";
        }

        if (g.getNumVertices() >= 2) {
            int u = 1, v = g.getNumVertices();
            std::cout << "Distancia entre " << u << " e " << v << ": "
                      << g.distance(u, v) << "\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
