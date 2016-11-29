import numpy as np
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit

fp = open('./data/freq1M')
vals = fp.read()
vals = vals.split('\n')
while vals[-1] == "":
    vals = vals[:-1]

for i in range(len(vals)):
    vals[i] = vals[i].split()
    vals[i] = int(vals[i][1])

vals = np.array(vals)


def inv_func(x, a, b):
    return 1 / (a * x + b)


def inv_func_new(x, a, b, pow):
    return 1 / (a * (x**pow) + b)


def quad_inv(x, a, b):
    return 1 / (a * x * x + b)


def exp(x, a, b, c):
    return a * np.exp(-b * x) + c


def new_func(x, a, b, c, d):
    return a*(b**(c*x))+d


y = vals
x = np.array(range(len(y))) + 1
x_new = np.linspace(x[0], x[-1], len(x))


params, _ = curve_fit(inv_func, x, y)
y_new = inv_func(x, params[0], params[1])
plt.plot(vals)
plt.plot(y_new)
plt.title("1/x")
plt.show()
print params

params, _ = curve_fit(inv_func_new, x, y)
y_new = inv_func_new(x, params[0], params[1], params[2])
plt.plot(vals)
plt.plot(y_new)
plt.title("1/(a*x^pow + b)")
plt.show()
print params

params, _ = curve_fit(quad_inv, x, y)
y_new = quad_inv(x, params[0], params[1])
plt.plot(vals)
plt.plot(y_new)
plt.title("1/(a*x^2 + b)")
plt.show()
print params

params, _ = curve_fit(exp, x, y)
y_new = exp(x, params[0], params[1], params[2])
plt.plot(vals)
plt.plot(y_new)
plt.title("1/(a*e^x + b)")
plt.show()
print params
