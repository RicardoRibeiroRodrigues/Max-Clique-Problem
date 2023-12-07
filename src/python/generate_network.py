import networkx as nx
import random
import sys

# Parâmetros
def generate_network(num_vertices: int, connection_chance: float, nome_arquivo: str = 'grafo.txt'):

    # Crie um grafo aleatório densamente conectado
    grafo = nx.fast_gnp_random_graph(num_vertices, connection_chance)

    # Abra o arquivo para escrita
    with open(nome_arquivo, 'w') as arquivo:
        # Escreva a quantidade de vértices e número de arestas na primeira linha
        arquivo.write(f"{num_vertices} {grafo.number_of_edges()}\n")

        # Escreva as arestas no formato de lista de adjacência
        for aresta in grafo.edges():
            arquivo.write(f"{aresta[0]+1} {aresta[1]+1}\n")  # +1 para ajustar os índices (começando em 1)

    print(f"Grafo densamente conectado com {num_vertices} vertices gerado e salvo em '{nome_arquivo}'.")

if __name__ == '__main__':
    num_vertices = 60  # Número de vértices no grafo
    if len(sys.argv) > 1:
        num_vertices = int(sys.argv[1])
    probabilidade_conexao = 0.7  # Probabilidade de haver uma aresta entre dois vértices (ajuste conforme necessário)
    generate_network(num_vertices, 0.7)