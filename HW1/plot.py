import matplotlib
import matplotlib.pyplot as plt
from numpy import genfromtxt
my_data = genfromtxt('color.txt', delimiter=' ')

plt.imshow(my_data, cmap = plt.cm.prism, interpolation = 'none')
plt.show()