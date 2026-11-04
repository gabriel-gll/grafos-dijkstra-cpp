# Biblioteca de Grafos — COS 242 (Teoria dos Grafos, 2026/2)

Biblioteca em C++17 para manipulação de grafos não-direcionados, com e sem pesos nas arestas, desenvolvida para o trabalho da disciplina de Teoria dos Grafos (UFRJ).

**Autores:** André Pacífico e Gabriel Lopes Lima

- **Parte 1:** representações (matriz e lista de adjacência), BFS, DFS, graus, componentes conexas, distância e diâmetro.
- **Parte 2:** grafos com pesos e caminhos mínimos com o algoritmo de Dijkstra, em duas versões (vetor e heap) escritas a partir de um único código.

## Funcionalidades

| Área | O que a biblioteca oferece |
|---|---|
| Representação | Matriz de adjacência ou lista de adjacência, escolhida ao criar o grafo |
| Pesos | Pesos reais (inclusive negativos) associados às arestas |
| Buscas | BFS e DFS (iterativa), com árvore de busca (pai e nível) |
| Estatísticas | Grau mínimo, máximo, médio e mediana; número e tamanho das componentes conexas |
| Distâncias | Distância em arestas (BFS), diâmetro exato e aproximado (*double sweep*) |
| Caminhos mínimos | Dijkstra com vetor ou com heap, árvore de caminhos mínimos e reconstrução do caminho |

## Estrutura do repositório

| Arquivo | Função |
|---|---|
| `Graph.hpp` / `Graph.cpp` | Classe `Graph` (a biblioteca) |
| `MemoryUtils.hpp` | Leitura da memória residente (RSS) do processo, via `/proc/self/status` |
| `main.cpp` | Programa de demonstração (`graph_program`) |
| `case_study.cpp` | Estudos de caso da Parte 1 (`case_study`) |
| `case_study2.cpp` | Estudos de caso da Parte 2 (`case_study2`) |
| `Makefile` | Compila os três programas |

## Requisitos e compilação

- `g++` com suporte a C++17
- Linux (a medição de memória do `MemoryUtils.hpp` usa `/proc/self/status`; em outros sistemas ela retorna 0, e o restante funciona normalmente)

```bash
make            # compila graph_program, case_study e case_study2
make clean      # remove objetos, executáveis e arquivos .txt gerados
```

## Formato do arquivo de entrada

A primeira linha tem o número de vértices `n`. Cada linha seguinte é uma aresta. Os vértices são numerados de 1 a `n`.

```
5
1 2 0.1
2 5 0.2
5 3 5
3 4 -9.5
4 5 2.3
1 5 1
```

- Com **duas colunas** (`u v`), o grafo é sem pesos e cada aresta vale 1.
- Com **três colunas** (`u v peso`), o grafo é ponderado. O formato é decidido pela primeira aresta lida.

## Uso dos programas

### `graph_program` — demonstração

```bash
./graph_program <arquivo> <matrix|list> [vertice_inicial]
```

Gera `output_info.txt` (vértices, arestas, graus e componentes), `output_bfs.txt` e `output_dfs.txt` (pai e nível de cada vértice) e, se o grafo não tiver pesos negativos, `output_dijkstra.txt` (distância e pai de cada vértice).

### `case_study` — estudos de caso da Parte 1

```bash
./case_study <arquivo> [--rep=matrix|list|both] [--threshold=N]
```

Mede memória e tempo médio de BFS/DFS (100 execuções), pai e nível na árvore de busca, distâncias, componentes conexas e diâmetro. O diâmetro exato só é calculado em grafos com até `N` vértices (padrão 20.000).

### `case_study2` — estudos de caso da Parte 2

```bash
./case_study2 <grafo> [--k=100] [--seed=42] [--rep=list|matrix] [--skip-vector]
./case_study2 <rede_colaboracao> --names=<arquivo_de_nomes>
```

| Opção | Efeito |
|---|---|
| `--k=N` | Número de origens aleatórias para medir o tempo (padrão 100) |
| `--seed=N` | Semente do sorteio das origens (padrão 42); as mesmas origens são usadas nas duas versões |
| `--rep=list\|matrix` | Representação usada (padrão: lista) |
| `--skip-vector` | Pula o Dijkstra com vetor, que é O(n²) e inviável em grafos grandes |
| `--names=arquivo` | Modo da rede de colaboração: distância e caminho de "Edsger W. Dijkstra" até os pesquisadores pedidos |

Sem `--names`, o programa calcula a distância e o caminho mínimo do vértice 10 aos vértices 20, 30, 40, 50 e 60 e mede o tempo médio do Dijkstra com heap e com vetor. O arquivo de nomes tem uma linha por vértice no formato `indice,nome`.

Exemplos:

```bash
./case_study2 grafo_W_3.txt
./case_study2 grafo_W_4.txt --skip-vector
./case_study2 rede_colaboracao.txt --names=rede_colaboracao_vertices.txt
```

## Usando a biblioteca no seu código

```cpp
#include "Graph.hpp"

Graph g(Graph::Representation::LIST);
g.loadFromFile("grafo.txt");

// Dijkstra a partir do vértice 10, com heap
Graph::ShortestPathTree tree = g.dijkstra(10, Graph::DijkstraMode::HEAP);

double d = tree.dist[20];                          // distância mínima de 10 até 20
std::vector<int> path = Graph::pathTo(tree, 20);   // caminho mínimo (vazio se inalcançável)
```

Principais métodos públicos da classe `Graph`:

| Método | Descrição |
|---|---|
| `loadFromFile(arquivo)` | Lê o grafo do arquivo |
| `isWeighted()`, `hasNegativeWeights()` | Informam o formato do grafo carregado |
| `degree`, `minDegree`, `maxDegree`, `averageDegree`, `medianDegree` | Estatísticas de grau |
| `bfs(start)`, `dfs(start)` | Árvore de busca (pai e nível); há sobrecarga que grava em arquivo |
| `connectedComponents()` | Componentes conexas, da maior para a menor |
| `distance(u, v)` | Distância em arestas (-1 se não houver caminho) |
| `diameter()`, `diameterApprox()` | Diâmetro exato (O(n·(n+m))) e aproximado (O(n+m)) |
| `dijkstra(start, modo)` | Árvore de caminhos mínimos; há sobrecarga que grava em arquivo |
| `weightedDistance(u, v, modo)` | Distância ponderada (infinito se não houver caminho) |
| `Graph::pathTo(tree, v)` | Reconstrói o caminho mínimo a partir da árvore |
| `writeGraphInfo(arquivo)` | Grava as informações gerais do grafo |

Com pesos negativos, `dijkstra()` lança uma exceção informando que a biblioteca ainda não implementa caminhos mínimos nesse caso.

## Decisões de projeto (Parte 2)

- **Pesos em estruturas paralelas.** Na lista, `adjWeights[u][i]` guarda o peso da aresta para `adjList[u][i]`; na matriz, `weightMatrix[u][v]` guarda o peso e a `adjMatrix` indica a existência da aresta. Elas só são alocadas em grafos ponderados.
- **Um único Dijkstra para as duas versões.** O algoritmo está em `runDijkstra<PQ>`, um template cujo parâmetro `PQ` é a estrutura que guarda as estimativas de distância. As classes `VectorQueue` e `HeapQueue` têm a mesma interface (`update(v, chave)` e `extractMin()`). O `runDijkstra` não sabe qual está usando; a escolha é feita em `dijkstra(start, mode)`.
- **`VectorQueue`:** `update` em O(1), `extractMin` varrendo os vértices em O(n); Dijkstra em O(n²).
- **`HeapQueue`:** heap binário indexado. O vetor `pos[v]` guarda a posição de `v` no heap, o que permite o *decrease-key* em O(log n) sem buscar o vértice; Dijkstra em O((n + m) log n).
- **`forEachWeightedNeighbor(v, fn)`:** percorre os vizinhos de `v` com seus pesos e chama `fn(vizinho, peso)`, sem copiar vetores.

## Limitações

- A matriz de adjacência é inviável nos grafos grandes dos estudos de caso (a memória cresce com n²); todos os estudos usam lista.
- O Dijkstra com vetor só foi medido no menor grafo ponderado; nos demais, o tempo é estimado pelo modelo O(n²).
- Em grafos com dezenas de milhões de vértices, a lista (dois `std::vector` por vértice) usa muita memória: o maior grafo testado chegou a cerca de 6,5 GiB de RAM.
- Pesos negativos não são suportados no cálculo de caminhos mínimos.

## Dados

Os grafos dos estudos de caso (`grafo_W_3` a `grafo_W_7` e a rede de colaboração) são grandes e foram disponibilizados no site da disciplina; eles não fazem parte deste repositório.
