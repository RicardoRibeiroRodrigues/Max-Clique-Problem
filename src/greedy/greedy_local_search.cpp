#include <iostream>
#include <vector>
#include <fstream>
#include <omp.h>
using namespace std;
#define NUMBER_OF_ITERATIONS 100000

typedef vector<vector<int>> Graph;

// Função para ler o grafo a partir do arquivo de entrada
std::vector<std::vector<int>> ReadGraph(const std::string &nomeArquivo, int &numVertices)
{
    std::ifstream arquivo(nomeArquivo);
    int numArestas;
    arquivo >> numVertices >> numArestas;

    std::vector<std::vector<int>> grafo(numVertices, std::vector<int>(numVertices, 0));

    for (int i = 0; i < numArestas; ++i)
    {
        int u, v;
        arquivo >> u >> v;
        grafo[u - 1][v - 1] = 1;
        grafo[v - 1][u - 1] = 1; // O grafo é não direcionado
    }

    arquivo.close();

    return grafo;
}

vector<int> find_maximal_clique(Graph &graph, int num_vertices)
{
    vector<int> maximal_clique;
    vector<int> candidates(num_vertices);
    for (int i = 0; i < num_vertices; i++)
        candidates[i] = i;
    
    while (!candidates.empty())
    {
        // Get random vertex within candidates
        int random_index = rand() % candidates.size();
        int v = candidates[random_index];
        candidates.erase(candidates.begin() + random_index);

        bool can_add = true;

        for (auto &u: maximal_clique)
        {
            if (graph[u][v] == 0)
            {
                can_add = false;
                break;
            }
        }

        if (can_add)
        {
            maximal_clique.push_back(v);
            vector<int> new_candidates;
            // Seleciona apenas canditados que pertencem ao clique atual.
            for (auto &u: candidates)
            {
                bool adjacent = true;

                for (auto &c: maximal_clique)
                {
                    if (graph[u][c] == 0)
                    {
                        adjacent = false;
                        break;
                    }
                }
                if (adjacent)
                    new_candidates.push_back(u);
            }
            candidates = new_candidates;
        }
    }
    return maximal_clique;
}

int main() 
{
    int n_vertices;
    Graph graph = ReadGraph("grafo.txt", n_vertices);

    // cout << "N threads: " << omp_get_max_threads() << endl;
    float time = omp_get_wtime();
    vector<int> max_clique, maximal_clique;
    for (int i = 0; i < NUMBER_OF_ITERATIONS; i++) {
        maximal_clique = find_maximal_clique(graph, n_vertices);
        if (maximal_clique.size() > max_clique.size()) {
            max_clique = maximal_clique;
        }
    }
    time = omp_get_wtime() - time;
    
    cout << "Time: " << time << " seconds" << endl;
    cout << "Found clique with size: " << max_clique.size() << endl;

    for (auto x: max_clique)
        cout << (x + 1) << " ";
    cout << endl;
}