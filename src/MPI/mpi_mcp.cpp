#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <omp.h>
#include <mpi.h>

using namespace std;

#define MASTER_RANK 0

struct MaxCliqueInfo
{
    int size;
    int rank;
};

typedef vector<vector<int>> Graph;

// Function to read the graph from the input file
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
        grafo[v - 1][u - 1] = 1; // The graph is undirected
    }

    arquivo.close();

    return grafo;
}

bool isClique(Graph &grafo, vector<int> &clique)
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

void rec_max_clique(int *grafo, int numVertices, vector<int> curr_clique, vector<int> &cantidates, vector<int> &maximal_clique, int StartVertex)
{
    // If there are no more candidates, check if the current clique is bigger than the maximal clique
    // and return
    if (cantidates.size() == 0)
    {
        if (curr_clique.size() > maximal_clique.size())
            maximal_clique = curr_clique;
        return;
    }

    // Optimization: if the current clique + the number of candidates is smaller than the max clique, return
    if (curr_clique.size() + cantidates.size() <= maximal_clique.size())
        return;

    // Not curr clique -> current clique without inserting the last vertex
    vector<int> not_curr_clique = curr_clique;
    // Insert the last vertex in the current clique
    int curr_vertex = cantidates.back();
    cantidates.pop_back();
    curr_clique.push_back(curr_vertex);

    // update the candidates -> only the vertices that are adjacent to all the vertices in the current clique
    vector<int> new_candidates;
    for (auto &x : cantidates)
    {
        bool is_adjacent = true;
        for (auto &y : curr_clique)
        {
            if (grafo[x * numVertices + y] == 0)
            {
                is_adjacent = false;
                break;
            }
        }
        if (is_adjacent)
        {
            new_candidates.push_back(x);
        }
    }
    // Two options: insert the vertex in the clique or not
    rec_max_clique(grafo, numVertices, curr_clique, new_candidates, maximal_clique, StartVertex);
    rec_max_clique(grafo, numVertices, not_curr_clique, cantidates, maximal_clique, StartVertex);
}

vector<int> find_max_clique(int *grafo, int numVertex, int startVertex, int endVertex)
{
    vector<int> clique, max_clique;

    vector<int> canditates(endVertex - startVertex);
    for (int i = startVertex; i < endVertex; i++)
        canditates[i - startVertex] = i;

    rec_max_clique(grafo, numVertex, clique, canditates, max_clique, startVertex);
    return max_clique;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, numProcesses;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

    float time = omp_get_wtime();

    int numVertices;
    int *graphArray;
    Graph graph;
    if (rank == MASTER_RANK)
    {
        graph = ReadGraph("grafo.txt", numVertices);
        // Transform the graph into an 1d array
        graphArray = new int[numVertices * numVertices];
        for (int i = 0; i < numVertices; i++)
        {
            for (int j = 0; j < numVertices; j++)
                graphArray[i * numVertices + j] = graph[i][j];
        }
        
        cout << "Master read the graph with " << graph.size() << " vertices" << endl;
    }

    // Broadcast the number of vertices to all processes
    MPI_Bcast(&numVertices, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    int verticesPerProcess = numVertices / numProcesses;
    int remainingVertices = numVertices % numProcesses;
    int startVertex = rank * verticesPerProcess + (rank < remainingVertices ? rank : remainingVertices);
    int endVertex = startVertex + verticesPerProcess + (rank < remainingVertices ? 1 : 0);

    if (rank != MASTER_RANK)
    {
        graphArray = new int[numVertices * numVertices];
    }

    // Broadcast the graph to all processes
    MPI_Bcast(graphArray, numVertices * numVertices, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
    cout << "Process " << rank << " received the graph" << endl;

    vector<int> maxClique;
    int maxCliqueSize = 0;

    vector<int> localClique = find_max_clique(graphArray, numVertices, startVertex, endVertex);
    cout << "Process " << rank << " found a clique of size " << localClique.size() << endl;
    MaxCliqueInfo localMaxCliqueInfo;
    localMaxCliqueInfo.size = localClique.size();
    localMaxCliqueInfo.rank = rank;
        
    // Reduce the local clique size to the master process
    MaxCliqueInfo globalMaxCliqueInfo;
    MPI_Allreduce(&localMaxCliqueInfo, &globalMaxCliqueInfo, 1, MPI_2INT, MPI_MAXLOC, MPI_COMM_WORLD);
    if (rank == globalMaxCliqueInfo.rank)
    {
        maxClique = localClique;
    } else {
        maxClique.resize(globalMaxCliqueInfo.size);
    }

    // Broadcast the global max clique size to all processes
    MPI_Bcast(maxClique.data(), globalMaxCliqueInfo.size, MPI_INT, globalMaxCliqueInfo.rank, MPI_COMM_WORLD);

    if (rank == MASTER_RANK)
    {
        time = omp_get_wtime() - time;
        if (!isClique(graph, maxClique))
        {
            cout << "------------------------ NOT CLIQUE ------------------------" << endl;
        }
    
        cout << "Max clique found in time: " << time << " seconds" << endl;
        cout << "Max clique size: " << maxClique.size() << endl;
        cout << "Max clique: ";
        for (auto &x : maxClique)
            cout << x << " ";
        cout << endl;
        // delete[] graphArray;
    }

    delete[] graphArray;
    MPI_Finalize();
    return 0;
}