CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

# Programa de demonstracao (uso basico da biblioteca)
DEMO_TARGET = graph_program
DEMO_SOURCES = main.cpp Graph.cpp
DEMO_OBJECTS = $(DEMO_SOURCES:.cpp=.o)

# Programa dos estudos de caso (memoria, tempo, diametro, componentes, etc.)
CASE_TARGET = case_study
CASE_SOURCES = case_study.cpp Graph.cpp
CASE_OBJECTS = case_study.o Graph.o

# Programa dos estudos de caso da Parte 2 (Dijkstra, grafos com pesos)
CASE2_TARGET = case_study2
CASE2_OBJECTS = case_study2.o Graph.o

all: $(DEMO_TARGET) $(CASE_TARGET) $(CASE2_TARGET)

$(DEMO_TARGET): $(DEMO_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(DEMO_TARGET) $(DEMO_OBJECTS)

$(CASE_TARGET): $(CASE_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(CASE_TARGET) $(CASE_OBJECTS)

$(CASE2_TARGET): $(CASE2_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(CASE2_TARGET) $(CASE2_OBJECTS)

%.o: %.cpp Graph.hpp MemoryUtils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(DEMO_TARGET) $(CASE_TARGET) $(CASE2_TARGET) *.txt

.PHONY: all clean
