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
vals = vals / float(sum(vals))


def inv(x, a):
    return a / x


def inv_func(x, a, b):
    return a / (x + b)


def zipf(x, a, b, pow):
    return a / np.power((x + b), pow)


def plain_zipf(x, a, pow):
    return a / np.power(x, pow)


def plain_exp(x, a, b):
    return a * np.exp(-b * x)


def inv_func_new(x, a, b, pow):
    return 1 / (a * (x**pow) + b)


def quad_inv(x, a, b):
    return 1 / (a * x * x + b)


def exp(x, a, b, c):
    return a * np.exp(-b * x) + c


def new_func(x, a, b, c, d):
    return a * (b**(c * x)) + d


y = vals
x = np.array(range(len(y))) + 1
x_new = np.linspace(x[0], x[-1], len(x))


params, err = curve_fit(inv, x, y)
y_new = inv(x, params[0])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("a/x")
plt.show()

params, err = curve_fit(plain_zipf, x, y)
y_new = plain_zipf(x, params[0], params[1])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("a/x^pow")
plt.show()

params, err = curve_fit(inv_func, x, y)
y_new = inv_func(x, params[0], params[1])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("a/(x+b)")
plt.show()

params, err = curve_fit(plain_exp, x, y)
y_new = plain_exp(x, params[0], params[1])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("a*e^-bx")
plt.show()


params, err = curve_fit(zipf, x, y)
y_new = zipf(x, params[0], params[1], params[2])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("a/(x+b)^pow")
plt.show()


params, err = curve_fit(inv_func_new, x, y)
y_new = inv_func_new(x, params[0], params[1], params[2])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("1/(a*x^pow + b)")
plt.show()

params, err = curve_fit(quad_inv, x, y)
y_new = quad_inv(x, params[0], params[1])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("1/(a*x^2 + b)")
plt.show()

params, err = curve_fit(exp, x, y)
y_new = exp(x, params[0], params[1], params[2])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("a*e^-bx + c")
plt.show()

params, err = curve_fit(new_func, x, y)
y_new = new_func(x, params[0], params[1], params[2], params[3])
print params
plt.plot(vals)
plt.plot(y_new)
plt.title("a*b^(cx) + d")
plt.show()
