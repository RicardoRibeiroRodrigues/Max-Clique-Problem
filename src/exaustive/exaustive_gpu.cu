#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sort.h>
#include <thrust/functional.h>
#include <thrust/transform.h>
#include <thrust/remove.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
using namespace std;

typedef vector<vector<int>> Graph;

bool isClique(Graph &graph, thrust::host_vector<int> &vertices)
{
    for (int i = 0; i < vertices.size(); ++i)
    {
        for (int j = i + 1; j < vertices.size(); ++j)
        {
            if (graph[vertices[i]][vertices[j]] == 0)
            {
                return false;
            }
        }
    }
    return true;
}

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

struct GreedyCliqueOperator
{
    int num_vertices;
    int* graph;
    int* clique;
    int clique_size;

    GreedyCliqueOperator(int* _graph, int _num_vertices, int _clique_size, int* _clique)
        : graph(_graph), num_vertices(_num_vertices), clique_size(_clique_size), clique(_clique) {}

    __host__ __device__ int operator()(int vertex) const
    {
        bool is_adjacent = true;
        // For each vertex in the clique
        for (int i = 0; i < clique_size; ++i)
        {
            int index = vertex * num_vertices + clique[i];
            // If the vertex is not adjacent to the current vertex
            if (graph[index] == 0)
            {
                is_adjacent = false;
                break;
            }
        }

        return is_adjacent ? vertex : -1;
    }
};

void debug_device_vector(thrust::device_vector<int> &vec)
{
    thrust::host_vector<int> vec_cpu = vec;
    for (int i = 0; i < vec_cpu.size(); ++i)
    {
        std::cout << vec_cpu[i] << " ";
    }
    std::cout << std::endl;
}


void mcp_gpu(thrust::device_vector<int> &gpu_vec, int &n_vertices, thrust::device_vector<int> &curr_clique, thrust::device_vector<int> &candidates, thrust::device_vector<int> &max_clique)
{
    if (candidates.size() == 0)
    {
        if (curr_clique.size() > max_clique.size())
        {
            max_clique = curr_clique;
        }
        return;
    }

    if (curr_clique.size() + candidates.size() <= max_clique.size())
    {
        return;
    }

    // Two options: add the first vertex in the candidates vector to the clique or not
    // Add the first vertex to the clique
    thrust::device_vector<int> not_curr_clique = curr_clique;
    curr_clique.push_back(candidates.back());
    // Remove the first vertex from the candidates vector
    candidates.pop_back();
    // Remove the vertices that are not adjacent to all vertices in the clique
    thrust::device_vector<int> new_candidates(candidates.size());
        
    thrust::transform(
        candidates.begin(), candidates.end(), new_candidates.begin(), 
        GreedyCliqueOperator(
            thrust::raw_pointer_cast(gpu_vec.data()), n_vertices,
            curr_clique.size(), thrust::raw_pointer_cast(curr_clique.data())
        )
    );
    // Remove the vertices that are not adjacent to all vertices in the clique
    new_candidates.erase(thrust::remove(new_candidates.begin(), new_candidates.end(), -1), new_candidates.end());

    mcp_gpu(gpu_vec, n_vertices, curr_clique, new_candidates, max_clique);
    mcp_gpu(gpu_vec, n_vertices, not_curr_clique, candidates, max_clique);
}

int main()
{
    int num_vertices;
    auto graph = ReadGraph("grafo.txt", num_vertices);
    // Convert graph to 1D host_vector
    thrust::host_vector<int> graph_cpu(num_vertices * num_vertices);
    for (int i = 0; i < num_vertices; ++i)
    {
        for (int j = 0; j < num_vertices; ++j)
        {
            graph_cpu[i * num_vertices + j] = graph[i][j];
        }
    }
    cout << "Grafo com " << num_vertices << " vértices" << endl;
    // Define the device vectors
    thrust::device_vector<int> graph_gpu = graph_cpu;
    thrust::device_vector<int> candidates_gpu(num_vertices);
    thrust::sequence(candidates_gpu.begin(), candidates_gpu.end());
    thrust::device_vector<int> max_clique_gpu;
    thrust::device_vector<int> curr_clique_gpu;

    auto start = std::chrono::high_resolution_clock::now();
    auto cand_end = candidates_gpu.end();
    mcp_gpu(graph_gpu, num_vertices, curr_clique_gpu, candidates_gpu, max_clique_gpu);
    thrust::host_vector<int> max_clique = max_clique_gpu;
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration in seconds
    std::chrono::duration<double> duration = end - start;
    double durationInSeconds = duration.count();

    // Print the results
    std::cout << "Maximum Clique Size: " << max_clique.size() << std::endl;

    std::cout << "Clique máxima encontrada em " << durationInSeconds << " Segundos" << endl;
    
    std::cout << "Maximum Clique: ";
    for (int i = 0; i < max_clique.size(); ++i)
    {
        std::cout << max_clique[i] + 1 << " ";
    }
    std::cout << std::endl;

    if (!isClique(graph, max_clique))
    {
        std::cout << "Error: the maximum clique is not a clique" << std::endl;
        return 1;
    }

    return 0;
}
