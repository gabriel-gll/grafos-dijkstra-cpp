#ifndef MEMORY_UTILS_HPP
#define MEMORY_UTILS_HPP

#include <fstream>
#include <sstream>
#include <string>

// ---------------------------------------------------------------------------
// getCurrentRSSKB
//
// Retorna a memoria residente (RSS - Resident Set Size) do processo atual,
// em KB, lendo o pseudo-arquivo /proc/self/status (padrao em sistemas Linux).
// RSS e a memoria que o processo esta de fato usando na RAM neste momento --
// e exatamente o que a dica do enunciado sugere medir ("verifique a memoria
// sendo utilizada pelo processo" logo depois de carregar o grafo).
//
// Se o arquivo nao existir ou a linha nao for encontrada (por exemplo, em
// sistemas que nao sejam Linux), retorna 0.
// ---------------------------------------------------------------------------
inline long getCurrentRSSKB() {
    std::ifstream statusFile("/proc/self/status");
    if (!statusFile.is_open()) {
        return 0;
    }

    std::string line;
    while (std::getline(statusFile, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            std::istringstream iss(line.substr(6));
            long kb = 0;
            if (iss >> kb) {
                return kb;
            }
            return 0;
        }
    }
    return 0;
}

inline double getCurrentRSSMB() {
    return getCurrentRSSKB() / 1024.0;
}

#endif // MEMORY_UTILS_HPP
