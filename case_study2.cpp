#include "Graph.hpp"
#include "MemoryUtils.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// case_study2 -- estudos de caso da Parte 2 (grafos com pesos / Dijkstra).
//
// Uso:
//   ./case_study2 <grafo> [--k=100] [--seed=42] [--rep=list|matrix]
//                 [--skip-vector] [--names=<arquivo_nomes>]
//
//  Perguntas 1 e 2 (todo grafo):
//    1. distancia e caminho minimo de 10 para 20,30,40,50,60 (tabela)
//    2. tempo medio de Dijkstra com vetor e com heap (k origens aleatorias)
//  Rede de colaboracao (se --names for passado):
//    3. distancia e caminho minimo de "Edsger W. Dijkstra" para os
//       pesquisadores pedidos.
//
// Arquivo de nomes: uma linha por vertice, "indice,nome" (ou "indice nome"
// ou "indice<TAB>nome"). Se a linha nao comecar com numero, o indice e o
// numero da linha.
// ---------------------------------------------------------------------------

using Clock = std::chrono::high_resolution_clock;

static void sep() {
    std::cout << "----------------------------------------------------------\n";
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n\"");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n\"");
    return s.substr(a, b - a + 1);
}

struct Names {
    std::vector<std::string> byIndex;          // byIndex[v] = nome
    std::map<std::string, int> byName;         // nome -> v
    bool loaded() const { return !byName.empty(); }
    std::string label(int v) const {
        if (v >= 0 && v < (int)byIndex.size() && !byIndex[v].empty()) return byIndex[v];
        return std::to_string(v);
    }
};

// O arquivo de nomes do site esta com "mojibake": acentos em UTF-8 foram
// lidos como Windows-1252 e regravados em UTF-8 (ex.: "Éva" virou "Ã‰va").
// Esta funcao desfaz isso: converte cada caractere de volta ao byte original
// (Windows-1252) e so aceita o resultado se for UTF-8 valido. Nomes ja
// corretos (ou so ASCII) passam sem alteracao.
static std::string fixMojibake(const std::string& s) {
    static const std::map<unsigned, unsigned char> cp1252 = {
        {0x20AC,0x80},{0x201A,0x82},{0x0192,0x83},{0x201E,0x84},{0x2026,0x85},
        {0x2020,0x86},{0x2021,0x87},{0x02C6,0x88},{0x2030,0x89},{0x0160,0x8A},
        {0x2039,0x8B},{0x0152,0x8C},{0x017D,0x8E},{0x2018,0x91},{0x2019,0x92},
        {0x201C,0x93},{0x201D,0x94},{0x2022,0x95},{0x2013,0x96},{0x2014,0x97},
        {0x02DC,0x98},{0x2122,0x99},{0x0161,0x9A},{0x203A,0x9B},{0x0153,0x9C},
        {0x017E,0x9E},{0x0178,0x9F}};
    std::string out;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char b = s[i];
        unsigned cp; int len;
        if (b < 0x80) { cp = b; len = 1; }
        else if ((b & 0xE0) == 0xC0) { cp = b & 0x1F; len = 2; }
        else if ((b & 0xF0) == 0xE0) { cp = b & 0x0F; len = 3; }
        else return s;
        if (i + len > s.size()) return s;
        for (int j = 1; j < len; ++j) {
            unsigned char cb = s[i + j];
            if ((cb & 0xC0) != 0x80) return s;
            cp = (cp << 6) | (cb & 0x3F);
        }
        if (cp < 0x80 || (cp >= 0xA0 && cp <= 0xFF)) out += (char)cp;
        else {
            auto it = cp1252.find(cp);
            if (it == cp1252.end()) return s;
            out += (char)it->second;
        }
        i += len;
    }
    // valida UTF-8 do resultado
    for (size_t k = 0; k < out.size();) {
        unsigned char b = out[k];
        int len = b < 0x80 ? 1 : (b & 0xE0) == 0xC0 ? 2 : (b & 0xF0) == 0xE0 ? 3
                  : (b & 0xF8) == 0xF0 ? 4 : 0;
        if (len == 0 || k + len > out.size()) return s;
        for (int j = 1; j < len; ++j)
            if (((unsigned char)out[k + j] & 0xC0) != 0x80) return s;
        k += len;
    }
    return out;
}

static Names loadNames(const std::string& filename, int n) {
    std::ifstream in(filename);
    if (!in.is_open()) throw std::runtime_error("Nao foi possivel abrir: " + filename);
    Names names;
    names.byIndex.assign(n + 1, "");
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (trim(line).empty()) continue;
        int idx = lineNo;
        std::string name = trim(line);
        size_t p = 0;
        while (p < line.size() && std::isdigit((unsigned char)line[p])) ++p;
        if (p > 0 && p < line.size() && (line[p] == ',' || line[p] == '\t' || line[p] == ' ')) {
            idx = std::stoi(line.substr(0, p));
            name = trim(line.substr(p + 1));
        }
        name = fixMojibake(name);
        if (idx >= 1 && idx <= n) {
            names.byIndex[idx] = name;
            names.byName[name] = idx;
        }
    }
    return names;
}

static std::string pathString(const std::vector<int>& path, const Names* names) {
    std::ostringstream os;
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) os << " -> ";
        os << (names ? names->label(path[i]) : std::to_string(path[i]));
    }
    return os.str();
}

static void printDistAndPath(const Graph::ShortestPathTree& tree, int target,
                             const Names* names) {
    std::vector<int> path = Graph::pathTo(tree, target);
    std::cout << "  destino " << (names ? names->label(target) : std::to_string(target))
              << " | distancia = ";
    if (path.empty()) {
        std::cout << "inf | caminho: (nao alcancavel)\n";
    } else {
        std::cout << tree.dist[target] << " | caminho (" << path.size() - 1
                  << " arestas): " << pathString(path, names) << "\n";
    }
}

static void question1(const Graph& g) {
    sep();
    std::cout << "Pergunta 1: distancia e caminho minimo de 10 para 20,30,40,50,60\n";
    sep();
    if (g.getNumVertices() < 60) {
        std::cout << "[Aviso] grafo com menos de 60 vertices; nao se aplica.\n";
        return;
    }
    Graph::ShortestPathTree tree = g.dijkstra(10, Graph::DijkstraMode::HEAP);
    for (int t : {20, 30, 40, 50, 60}) printDistAndPath(tree, t, nullptr);
}

static void question2(const Graph& g, int k, unsigned seed, bool skipVector) {
    sep();
    std::cout << "Pergunta 2: tempo medio de Dijkstra (" << k << " origens aleatorias, seed "
              << seed << ")\n";
    sep();
    int n = g.getNumVertices();
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> pick(1, n);
    std::vector<int> sources(k);
    for (int& s : sources) s = pick(rng);

    auto timeMode = [&](Graph::DijkstraMode mode) {
        double total = 0.0;
        for (int s : sources) {
            auto t0 = Clock::now();
            auto tree = g.dijkstra(s, mode);
            auto t1 = Clock::now();
            (void)tree;
            total += std::chrono::duration<double, std::milli>(t1 - t0).count();
        }
        return total / k;
    };

    std::cout << std::fixed << std::setprecision(3);

    // 1) Heap primeiro; o resultado aparece no terminal na hora (flush),
    //    antes de comecar o Dijkstra com vetor, que pode demorar muito.
    double heapMs = timeMode(Graph::DijkstraMode::HEAP);
    std::cout << "Tempo medio Dijkstra com heap : " << heapMs << " ms" << std::endl;

    if (skipVector) {
        std::cout << "Tempo medio Dijkstra com vetor: (pulado por --skip-vector)\n";
    } else {
        // 2) Vetor depois.
        std::cout << "Rodando Dijkstra com vetor, pode demorar..." << std::endl;
        double vecMs = timeMode(Graph::DijkstraMode::VECTOR);
        std::cout << "Tempo medio Dijkstra com vetor: " << vecMs << " ms" << std::endl;
        if (heapMs > 0) std::cout << "Razao vetor/heap: " << vecMs / heapMs << "x\n";

        // Sanidade (fora da medicao, so depois de tudo): as duas
        // implementacoes devem concordar.
        {
            auto a = g.dijkstra(sources[0], Graph::DijkstraMode::VECTOR);
            auto b = g.dijkstra(sources[0], Graph::DijkstraMode::HEAP);
            bool same = true;
            for (int v = 1; v <= n && same; ++v) {
                if (std::fabs(a.dist[v] - b.dist[v]) > 1e-9 * (1 + std::fabs(a.dist[v])) &&
                    !(std::isinf(a.dist[v]) && std::isinf(b.dist[v]))) same = false;
            }
            std::cout << "Checagem: distancias vetor == heap para a 1a origem? "
                      << (same ? "sim" : "NAO") << "\n";
        }
    }
}

static void questionResearchers(const Graph& g, const Names& names) {
    sep();
    std::cout << "Rede de colaboracao: Edsger W. Dijkstra -> pesquisadores\n";
    sep();
    const std::string source = "Edsger W. Dijkstra";
    auto it = names.byName.find(source);
    if (it == names.byName.end()) {
        std::cout << "[Erro] \"" << source << "\" nao encontrado no arquivo de nomes.\n";
        return;
    }
    std::cout << "Indice de \"" << source << "\": " << it->second << "\n";
    Graph::ShortestPathTree tree = g.dijkstra(it->second, Graph::DijkstraMode::HEAP);
    for (const char* name : {"Alan M. Turing", "J. B. Kruskal", "Jon M. Kleinberg",
                                    "\xC3\x89va Tardos", "Daniel R. Figueiredo"}) {
        auto f = names.byName.find(name);
        if (f == names.byName.end() && std::string(name) == "\xC3\x89va Tardos") {
            // plano B: arquivo de nomes sem acento
            f = names.byName.find("Eva Tardos");
        }
        if (f == names.byName.end()) {
            std::cout << "  \"" << name << "\" nao encontrado no arquivo de nomes.\n";
            continue;
        }
        printDistAndPath(tree, f->second, &names);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0]
                  << " <grafo> [--k=100] [--seed=42] [--rep=list|matrix]"
                     " [--skip-vector] [--names=arquivo]\n";
        return 1;
    }
    std::string filename = argv[1], namesFile;
    int k = 100;
    unsigned seed = 42;
    bool skipVector = false;
    Graph::Representation rep = Graph::Representation::LIST;

    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        if (a.rfind("--k=", 0) == 0) k = std::stoi(a.substr(4));
        else if (a.rfind("--seed=", 0) == 0) seed = (unsigned)std::stoul(a.substr(7));
        else if (a.rfind("--names=", 0) == 0) namesFile = a.substr(8);
        else if (a == "--skip-vector") skipVector = true;
        else if (a == "--rep=matrix") rep = Graph::Representation::MATRIX;
        else if (a == "--rep=list") rep = Graph::Representation::LIST;
        else { std::cerr << "Argumento nao reconhecido: " << a << "\n"; return 1; }
    }

    try {
        Graph g(rep);
        g.loadFromFile(filename);
        std::cout << "Grafo: " << g.getNumVertices() << " vertices, " << g.getNumEdges()
                  << " arestas, " << (g.isWeighted() ? "com pesos" : "SEM pesos")
                  << (g.hasNegativeWeights() ? " (pesos NEGATIVOS)" : "") << "\n";
        if (g.hasNegativeWeights()) {
            std::cout << "A biblioteca ainda nao implementa caminhos minimos com "
                         "pesos negativos.\n";
            return 0;
        }
        if (g.getNumVertices() < 1) return 0;

        if (namesFile.empty()) {
            question1(g);
            question2(g, k, seed, skipVector);
        } else {
            Names names = loadNames(namesFile, g.getNumVertices());
            questionResearchers(g, names);
        }
        sep();
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
