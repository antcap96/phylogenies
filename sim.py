"""
create an example
saved input to in2
saved expected output to check2
"""

import numpy as np
from scipy.sparse.csgraph import minimum_spanning_tree
import sys

size = 100
str_size = 70
max_diff = 25

if len(sys.argv) > 1:
    if len(sys.argv) != 4:
        print("need to provide 3 arguments")
        exit()
    size, str_size, max_diff = [int(i) for i in sys.argv[1:]]

data = np.random.randint(2, size=(size, str_size))

with open("in2", "w") as f:
    print(size, file=f)
    for i in range(size):
        print(*data[i], sep='', file=f)
    print(max_diff, file=f)

graph = np.zeros((size, size))

for i in range(size-1):
    for j in range(i+1, size):
        graph[i, j] = np.sum(data[i] != data[j]) + i / size + j / size**2

graph[ graph >= max_diff+1] = 0

graph = minimum_spanning_tree(graph, overwrite=True)

with open("check2", "w") as f:
    for i in range(size-1):
        for j in range(i+1, size):
            if graph[i, j] != 0:
                print(i+1 ,j+1, file=f)
