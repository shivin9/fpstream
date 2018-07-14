import numpy as np
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit
from sklearn.metrics import r2_score as r2

fp = open('./data/freq1M')
vals = fp.read()
vals = vals.split('\n')
while vals[-1] == "":
    vals = vals[:-1]

for i in range(len(vals)):
    vals[i] = vals[i].split()
    vals[i] = int(vals[i][1])

vals = np.array(vals)
# vals = vals / 98418.0
vals = vals / 985043.0

def inv(x, a):
    return a / x


def inv_func(x, a, b):
    return a / (x + b)


def geometric(x, a, b):
    return a * np.power(b, x)


def zipf(x, a, b, pow):
    return a / np.power((x + b), pow)


def plain_zipf(x, a, pow):
    return a / np.power(x, pow)


def power(x, a, pow):
    return a * np.power(x, pow)


def plain_exp(x, a, b):
    return a * np.exp(-b * x)


def power_exp(x, a, b, pow):
    return a * np.exp(-b * np.power(x,pow))


def sum_exp(x, a, b, c, d):
    return a * np.exp(-b * x) + c * np.exp(-d * x)


def forced_exp(x, a):
    # return  (vals[0]/np.exp(-0.05729356)) * np.exp(-a * x)
    return  vals[0] * np.exp(-a * x)


def inv_func_new(x, a, b, pow):
    return 1 / (a * (x**pow) + b)


def quad_inv(x, a, b):
    return 1 / (a * x * x + b)


def exp(x, a, b, c):
    return a * np.exp(-b * x) + c


def inv_exp(x, a, b, c):
    return a / (b * np.exp(c * x) + 1)


def new_func(x, a, b, c, d):
    return a * (b**(c * x)) + d



def expo_wt(y, weight):
    new = np.array(0)
    # new = []
    a, b = len(y)/2, len(y)
    wt = 1

    while a < b:
        new = np.hstack((np.repeat(y[a:b], wt), new))
        b = a
        a = a/2
        wt *= weight
    return new[:-1]

weights = np.zeros(100)


y = vals
x = np.array(range(len(y)))
x_new = np.linspace(x[0], x[-1], len(x))
# x_new = expo_wt(x_new)
# y = expo_wt(y)
# vals = expo_wt(vals)

# params, err = curve_fit(inv, x, y)
# y_new = inv(x, params[0])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("a/x")
# plt.show()

params, err = curve_fit(geometric, x, y)
# y_new = geometric(x, params[0], params[1], params[2])
y_new = geometric(x, params[0], params[1])
print params, r2(y_new, vals)
plt.plot(vals)
plt.plot(y_new)
plt.title("a*b^x (geometric)")
plt.show()

# params, err = curve_fit(plain_zipf, x, y)
# y_new = plain_zipf(x, params[0], params[1])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("a/x^pow")
# plt.show()

params, err = curve_fit(inv_func, x, y)
y_new = inv_func(x, params[0], params[1])
print params, r2(y_new, vals)
plt.plot(vals)
plt.plot(y_new)
plt.title("a*b^(c*x)")
plt.show()

# params, err = curve_fit(plain_exp, x_new, y)
# y_new = plain_exp(x, params[0], params[1])
# print params, r2(y_new, vals)
# plt.plot(y)
# plt.plot(y_new)
# plt.title("a*e^bx")
# plt.show()

params, err = curve_fit(power_exp, x_new, y)
y_new = power_exp(x, params[0], params[1], params[2])
print params, r2(y_new, vals)
plt.plot(y)
plt.plot(y_new)
plt.title("a*e^-b(x^pow)")
plt.show()

# params, err = curve_fit(zipf, x, y)
# y_new = zipf(x, params[0], params[1], params[2])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("a/(x+b)^pow")
# plt.show()


# params, err = curve_fit(inv_func_new, x, y)
# y_new = inv_func_new(x, params[0], params[1], params[2])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("1/(a*x^pow + b)")
# plt.show()

# params, err = curve_fit(quad_inv, x, y)
# y_new = quad_inv(x, params[0], params[1])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("1/(a*x^2 + b)")
# plt.show()

# params, err = curve_fit(exp, x, y)
# y_new = exp(x, params[0], params[1], params[2])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("a*e^-bx + c")
# plt.show()

# params, err = curve_fit(inv_exp, x, y)
# y_new = inv_exp(x, params[0], params[1], params[2])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("a/(b*e^cx + 1)")
# plt.show()

# params, err = curve_fit(new_func, x, y)
# y_new = new_func(x, params[0], params[1], params[2], params[3])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("a*b^(cx) + d")
# plt.show()

# params, err = curve_fit(sum_exp, x, y)
# y_new = sum_exp(x, params[0], params[1], params[2], params[3])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("sum exp")
# plt.show()

# params, err = curve_fit(forced_exp, x, y)
# y_new = forced_exp(x, params[0])
# print params, r2(y_new, vals)
# plt.plot(vals)
# plt.plot(y_new)
# plt.title("force exp")
# plt.show()

# import  numpy as np

# gnd = open("./tests/100kD100T10S40.gnd","r")

# trans = gnd.read()
# trans = trans.split("\n")

# for i in range(len(trans)):
#     trans[i] = trans[i].split(" ")

# lambdas = np.zeros(100)
# freqs = np.zeros(100, dtype='int')

# for i in range(len(trans)):   
#     if(len(trans[i]) == 2):
#         lambdas[int(trans[i][0])] = float(trans[i][-1][1:-1])/100.0
#     #trans[i] = trans[i][0:-1]
#     freqs[len(trans[i])] += 1

# test = open("./tests/100kD100T10.tab","r")
# trans = test.read()
# trans = trans.split("\n")

# for i in range(len(trans)):
#     trans[i] = trans[i].strip()
#     trans[i] = trans[i].split(" ")

# one_itemset = np.zeros(100, dtype='int')
# for i in range(len(trans)):
#     for j in trans[i][1:-1]:
#         one_itemset[int(j)] += 1


# c0, c1 = 0.41809927, 7.90258727 # 10k
# c0, c1 = 0.36478481, 5.11268154 # 1M
# c0, c1 = 2.78474589, 6.1196634  # 100k
# c0, c1 = 0.32350447, 0.04494187 # 100k exp

# def item_freq(x, c0=c0,c1=c1):
#     return c0/(c1+x)

# def item_freq(x, c0=c0,c1=c1):
#     return c0*np.exp(-c1*x)

# def freqs(list1):             
#     prod = 1.0      
#     for i in list1:
#         prod *= lambdas[i]
#     return prod*100.0

# def pattern_count(k, D, sigma=0.05, c0=c0, c1=c1):
#      ik = (c1/sigma)*(np.power(c0/c1, k) - sigma)
#      return np.power(ik,k)/np.power(factorial(k),2)

# def Ik(c1, c2, k, sigma=0.05):                    
#     return (c2/sigma)*(np.power(c1/c2, k) - sigma)

# def min1(a,b):                                    
#      inner = (1+np.exp(2*(a-b)))/2                        
#      return a - 0.5*np.log(inner)