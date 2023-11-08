#include <iostream>
#include <vector>
#include <fstream>
#include <omp.h>
using namespace std;

typedef vector<vector<int>> Graph;

std::vector<std::vector<int>> LerGrafo(const std::string &nomeArquivo, int &numVertices)
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

vector<int> max(vector<int> c_1, vector<int> c_2)
{
    if (c_1.size() > c_2.size())
        return c_1;
    else
        return c_2;
}

vector<int> rec_max_clique(Graph &grafo, int curr_vertex, vector<int> curr_clique)
{
    if (curr_vertex == grafo.size())
        return curr_clique;
    // Here we have two options:
    // 1. Add the vertex to the clique
    // 2. Don't add the vertex to the clique
    // We will try both options and return the clique with the maximum size
    vector<int> not_curr_clique = curr_clique;
    // First we check if the vertex is adjacent to all the vertices in the clique
    bool is_adjacent = true;
    for (auto &x: curr_clique)
    {
        if (grafo[curr_vertex][x] == 0)
        {
            is_adjacent = false;
            break;
        }
    }
    if (is_adjacent)
    {
        curr_clique.push_back(curr_vertex);
        vector<int> clique_1;
        vector<int> clique_2;
        clique_1 = rec_max_clique(grafo, curr_vertex + 1, curr_clique);
        clique_2 = rec_max_clique(grafo, curr_vertex + 1, not_curr_clique);
        return max(clique_1, clique_2);
    } else {
        return rec_max_clique(grafo, curr_vertex + 1, not_curr_clique);
    }
}

vector<int> find_max_clique(Graph &grafo, int n_vertices)
{
    vector<int> clique;
    return rec_max_clique(grafo, 0, clique);
}

int main()
{
    int n_vertices;
    float time = omp_get_wtime();
    Graph grafo = LerGrafo("grafo.txt", n_vertices);
    std::cout << "Grafo com " << n_vertices << " vertices" << endl;
    vector<int> max_clique;

    max_clique = find_max_clique(grafo, n_vertices);
    time = omp_get_wtime() - time;
    // 2.5~2.7 com media em 2.5 segundos.
    std::cout << "Clique máxima encontrada em " << time << " Segundos" << endl;

    std::cout << "Max clique with size: " << max_clique.size() << endl;
    for (auto &x: max_clique)
        std::cout << x + 1 << " ";
    std::cout << endl;
    return 0;
}