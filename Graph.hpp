#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <limits>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Graph
//
// Biblioteca para manipulacao de grafos nao-direcionados.
// Suporta duas representacoes internas (o usuario escolhe qual usar):
//   - MATRIX : matriz de adjacencia (vector<vector<int>>, 0/1)
//   - LIST   : lista de adjacencia (vector<vector<int>>, vizinhos de cada v.)
//
// Vertices sao numerados de 1 a n (o vertice 0 nao e usado, apenas para
// simplificar a indexacao dos vetores).
// ---------------------------------------------------------------------------
class Graph {
public:
    enum class Representation { MATRIX, LIST };

    // Estrutura de retorno para BFS/DFS: para cada vertice, guarda o pai na
    // arvore de busca (0 se for a raiz, -1 se o vertice nao foi visitado) e o
    // nivel na arvore (-1 se nao visitado).
    struct SearchTree {
        std::vector<int> parent;
        std::vector<int> level;
    };

    // Resultado da busca de componentes conexas: cada componente e uma lista
    // de vertices. O vetor externo vem ordenado em ordem decrescente de
    // tamanho (maior componente primeiro).
    using Components = std::vector<std::vector<int>>;

    // ---- Parte 2: caminhos minimos -----------------------------------------
    // Resultado de Dijkstra: dist[v] = distancia minima da origem a v
    // (INFINITY se inalcancavel); parent[v] = pai de v na arvore de caminhos
    // minimos (0 para a origem, -1 se inalcancavel).
    struct ShortestPathTree {
        int source = 0;
        std::vector<double> dist;
        std::vector<int> parent;
    };

    // Como as estimativas de distancia sao armazenadas / o minimo e extraido:
    //   VECTOR : vetor simples, extracao do minimo por varredura -- O(n^2)
    //   HEAP   : heap binario indexado com decrease-key        -- O((n+m) log n)
    enum class DijkstraMode { VECTOR, HEAP };

    static constexpr double INF = std::numeric_limits<double>::infinity();

    explicit Graph(Representation rep = Representation::LIST);

    // ---- Construcao / entrada -------------------------------------------
    // Le o grafo a partir de um arquivo texto no formato descrito no
    // enunciado: primeira linha = numero de vertices; linhas seguintes = uma
    // aresta "u v" por linha.
    void loadFromFile(const std::string& filename);

    // ---- Informacoes basicas ---------------------------------------------
    // O arquivo pode ter 2 colunas ("u v", grafo sem pesos; cada aresta vale
    // 1) ou 3 colunas ("u v peso"). O formato e decidido pela primeira aresta.
    bool isWeighted() const { return weighted; }
    bool hasNegativeWeights() const { return negativeWeights; }
    int getNumVertices() const { return numVertices; }
    int getNumEdges() const { return numEdges; }
    Representation getRepresentation() const { return representation; }

    int degree(int v) const;
    int minDegree() const;
    int maxDegree() const;
    double averageDegree() const;
    double medianDegree() const;

    // Escreve em um arquivo texto: numero de vertices, numero de arestas,
    // grau minimo, maximo, medio, mediana e informacoes das componentes
    // conexas (quantidade, tamanho de cada uma e vertices pertencentes).
    void writeGraphInfo(const std::string& filename) const;

    // ---- Buscas -------------------------------------------------------
    // Executa BFS/DFS a partir de "start" e grava a arvore de busca
    // (pai e nivel de cada vertice) no arquivo indicado.
    SearchTree bfs(int start) const;
    SearchTree dfs(int start) const;

    void bfs(int start, const std::string& outFilename) const;
    void dfs(int start, const std::string& outFilename) const;

    // ---- Distancias e diametro --------------------------------------------
    // Distancia (numero de arestas do caminho minimo) entre u e v.
    // Retorna -1 se nao houver caminho entre eles.
    int distance(int u, int v) const;

    // Diametro exato: maior distancia entre qualquer par de vertices.
    // Custo: O(n * (n + m)) -- BFS a partir de cada vertice.
    int diameter() const;

    // Diametro aproximado (heuristica de "double sweep"): faz uma BFS a
    // partir de um vertice arbitrario para achar o vertice mais distante a;
    // faz uma segunda BFS a partir de a para achar o vertice mais distante b;
    // retorna dist(a, b) como aproximacao do diametro. Custo: O(n + m).
    int diameterApprox() const;

    // ---- Componentes conexas ----------------------------------------------
    Components connectedComponents() const;

    // ---- Caminhos minimos (Dijkstra) --------------------------------------
    // Distancia e arvore de caminhos minimos de "start" para todos os
    // vertices. Lanca std::runtime_error se o grafo tiver pesos negativos
    // (a biblioteca ainda nao implementa esse caso).
    // Ambos os modos usam o MESMO codigo de Dijkstra; so muda a estrutura
    // que guarda as estimativas e extrai o minimo.
    ShortestPathTree dijkstra(int start, DijkstraMode mode = DijkstraMode::HEAP) const;

    // Grava "vertice distancia pai" para todos os vertices.
    void dijkstra(int start, const std::string& outFilename,
                  DijkstraMode mode = DijkstraMode::HEAP) const;

    // Distancia ponderada de u a v (INF se nao houver caminho).
    double weightedDistance(int u, int v, DijkstraMode mode = DijkstraMode::HEAP) const;

    // Caminho minimo origem -> target extraido da arvore (vazio se inalcancavel).
    static std::vector<int> pathTo(const ShortestPathTree& tree, int target);

private:
    int numVertices;
    int numEdges;
    Representation representation;

    std::vector<std::vector<int>> adjMatrix; // usado se representation == MATRIX
    std::vector<std::vector<int>> adjList;   // usado se representation == LIST

    // Pesos (so alocados se o grafo for com pesos):
    //   MATRIX: weightMatrix[u][v] vale o peso se adjMatrix[u][v] == 1
    //   LIST  : adjWeights[u][i] e o peso da aresta para adjList[u][i]
    // (adjMatrix/adjList continuam intactos, entao a Parte 1 nao muda.)
    std::vector<std::vector<double>> weightMatrix;
    std::vector<std::vector<double>> adjWeights;
    bool weighted = false;
    bool negativeWeights = false;

    void addEdge(int u, int v, double w = 1.0);
    std::vector<int> neighbors(int v) const;

    // BFS auxiliar usada por distance()/diameter(); retorna o vetor de
    // niveis (distancias) a partir de "start" (-1 para vertices nao
    // alcancados).
    std::vector<int> bfsLevels(int start) const;

    // Chama fn(vizinho, peso) para cada aresta de v, sem copiar vizinhos.
    template <typename Fn>
    void forEachWeightedNeighbor(int v, Fn fn) const;

    template <typename PQ>
    ShortestPathTree runDijkstra(int start) const;
};

#endif // GRAPH_HPP
