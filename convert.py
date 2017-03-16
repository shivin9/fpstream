import os

def data_to_tab():
    # fname = raw_input("enter name of file\n")
    # fname = "./data/" + fname

    os.chdir("./data")
    for file in os.listdir('.'):
        if file[-5:] == ".data":
            with open(file, 'r') as f:
                vals = f.read()
            print "Converting file " + file

            vals = vals.split("\n")

            for i in range(len(vals)):
                vals[i] = vals[i].split(" ")

            for i in range(len(vals)):
                vals[i] = vals[i][1:]
                vals[i] = vals[i][:-1]

            new = ""

            for i in range(len(vals)):
                temp = ""
                temp = ", ".join(list(set(vals[i]))) + "\n"
                new += temp

            out = open("../tests/" + file[:-5]+".tab", "w")
            print >> out, new

    os.chdir("../")


def main():
    fname = raw_input("enter name of file\n")
    #fname = "./data/" + fname
    with open(fname, 'r') as f:
        vals = f.read()
    vals = vals.split("\r\n")
    for i in range(len(vals)):
        vals[i] = vals[i].split(" ")
    new = ""
    temp = ""
    i = 0
    print vals
    vals = vals[:-1]
    while i < len(vals):
        frst = vals[i][0]
        cnt = 0
        temp = []
        while i < len(vals) and vals[i][0] == frst:
            cnt += 1
            temp.append(vals[i][1])
            temp = temp + vals[i][1] + ","
            i += 1
            print temp
        temp = list(set(temp))
        temp = " ".join(temp)
        # temp = str(cnt) + " " + temp + "\n"
        temp = temp + "\n"
        print temp
        new += temp

    # out = open(fname+".data", "w")
    out = open(fname+".tab", "w")
    print >> out, new

if __name__ == '__main__':
    data_to_tab()
    # main()
