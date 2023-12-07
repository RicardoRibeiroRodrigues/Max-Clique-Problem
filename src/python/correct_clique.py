import networkx as nx
import sys

# Nome do arquivo de entrada
nome_arquivo = "grafo.txt"
verbose = False
if len(sys.argv) > 1:
    nome_arquivo = sys.argv[1]
    if len(sys.argv) > 2:
        verbose = sys.argv[2] == '-v'

# Abrir o arquivo e pular a primeira linha
with open(nome_arquivo, 'r') as arquivo:
    next(arquivo)  # Pula a primeira linha

    # Lê o grafo a partir das linhas restantes
    G = nx.parse_adjlist(arquivo)

# Encontrar todas as cliques maximais
cliques_maximais = list(nx.find_cliques(G))

# Encontrar a clique máxima (a maior)
clique_maxima = max(cliques_maximais, key=len)

if verbose:
    print("Cliques maximais encontradas:")
    for clique in cliques_maximais:
        print(clique)

print("Tamanho clique max: ", len(clique_maxima))
print("Clique máxima encontrada:", clique_maxima)