#include <iostream>
#include <vector>
#include <fstream>
#include <omp.h>
#include <map>
#include <algorithm>
using namespace std;

typedef vector<vector<int>> Graph;

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

bool isClique(Graph &grafo, vector<int> clique)
{
    for (int i = 0; i < (int) clique.size(); i++)
    {
        for (int j = i + 1; j < (int) clique.size(); j++)
        {
            if (grafo[clique[i]][clique[j]] == 0)
                return false;
        }
    }
    return true;
}

void rec_max_clique(Graph &grafo, vector<int> curr_clique, vector<int>&cantidates, vector<int>&maximal_clique)
{
    // If there are no more candidates, check if the current clique is bigger than the maximal clique
    // and return
    if (cantidates.size() == 0) {
        #pragma omp critical
        {
            if (curr_clique.size() > maximal_clique.size()){
                maximal_clique = curr_clique;
            }
        }
        return;
    }

    // Optimization: if the current clique + the number of candidates is smaller than the max found clique, return
    if (curr_clique.size() + cantidates.size() <= maximal_clique.size())
        return;
    
    // Not curr clique -> current clique without inserting the last vertex 
    vector<int> not_curr_clique = curr_clique;
    // Insert the last vertex in the current clique
    int curr_vertex = cantidates.back();
    cantidates.pop_back();
    curr_clique.push_back(curr_vertex);

    // Update the candidates -> only the vertices that are adjacent all the vertices in the current clique
    vector<int> new_candidates;
    for (auto &x: cantidates)
    {
        bool is_adjacent = true;
        for (auto &y: curr_clique)
        {
            if (grafo[x][y] == 0)
            {
                is_adjacent = false;
                break;
            }
        }
        if (is_adjacent) {
            new_candidates.push_back(x);
        }
    }

    // Limiting Parallelization Depth: if the number of candidates is bigger than the threshold, parallelize the recursion
    // If this is not done, the overhead of creating threads will be bigger than the gain of parallelization (experimentally tested).
    const int parallelization_threshold = 60;
    if ((int) cantidates.size() >= parallelization_threshold) {
        #pragma omp parallel
        {
            #pragma omp task shared(maximal_clique)
                rec_max_clique(grafo, curr_clique, new_candidates, maximal_clique);
            #pragma omp task shared(maximal_clique)
                rec_max_clique(grafo, not_curr_clique, cantidates, maximal_clique);
        }
    } else {
        rec_max_clique(grafo, curr_clique, new_candidates, maximal_clique);
        rec_max_clique(grafo, not_curr_clique, cantidates, maximal_clique);
    }
}

// Function to initialize the recursion and return the maximum clique
vector<int> find_max_clique(Graph &grafo)
{
    vector<int> clique, max_clique;

    vector<int> canditates(grafo.size());
    for (int i = 0; i < (int) grafo.size(); i++)
        canditates[i] = i;

    rec_max_clique(grafo, clique, canditates, max_clique);
    return max_clique;
}

int main()
{
    int n_vertices;
    Graph graph = ReadGraph("grafo.txt", n_vertices);
    std::cout << "Grafo com " << n_vertices << " vertices" << endl;

    float time = omp_get_wtime();
    vector<int> result = find_max_clique(graph);
    time = omp_get_wtime() - time;
    if (!isClique(graph, result))
    {
        cout << "Error: the result is not a clique" << endl;
        return 1;
    }
    cout << "Size of maximum clique: " << result.size() << endl;
    std::cout << "Clique máxima encontrada em " << time << " Segundos" << endl;

    // std::cout << "Max clique with size: " << max_clique.size() << endl;
    for (auto &x: result)
        std::cout << x + 1 << " ";
    std::cout << endl;
    return 0;
}