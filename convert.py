import os

def convert_file(file):
    if file[-5:] == ".data":
        with open(file, 'r') as f:
            vals = f.read()

        print "Converting file " + file

        if file[0:7] == "./data/":
            file = file[7:]

        vals = vals.split("\n")

        for i in range(len(vals)):
            vals[i] = vals[i].split(" ")

        for i in range(len(vals)):
            vals[i] = vals[i][1:]
            # vals[i] = vals[i][:-1]

        new = ""

        for i in range(len(vals)):
            temp = ""
            temp = " ".join(list(set(vals[i]))) + "\n"
            new += temp

        out = open("./tests/" + file[:-5]+".tab", "w")
        print >> out, new


def data_to_tab():
    fname = raw_input("enter [relative] name of file\n")
    if fname != "all":
        convert_file("./data/" + fname)
    
    else:
        os.chdir("./data")
        for file in os.listdir('.'):
            os.chdir("../")
            convert_file(file)
            os.chdir("./data")


def main():
    fname = raw_input("enter name of file\n")
    # fname = "./data/" + fname
    with open(fname, 'r') as f:
        vals = f.read()
    vals = vals.split("\r\n")
    for i in range(len(vals)):
        vals[i] = vals[i].split(" ")
    new = ""
    temp = ""
    i = 0
    # print vals
    vals = vals[:-1]
    while i < len(vals):
        frst = vals[i][0]
        temp = []
        while i < len(vals) and vals[i][0] == frst:
            temp.append(vals[i][1])
            # temp = temp + vals[i][1] + ","
            i += 1
            # print temp
        temp = list(set(temp))
        cnt = len(temp)
        temp = " ".join(temp)
        temp = str(cnt) + " " + temp + "\n"
        # temp = temp + "\n"
        # print temp
        new += temp

    fname = fname.split(".")[0]
    out = open(fname + ".data", "w")
    # out = open(fname + ".tab", "w")
    print >> out, new


if __name__ == '__main__':
    # data_to_tab()
    main()
