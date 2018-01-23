<<<<<<< HEAD
def data_to_tab():
    fname = raw_input("enter name of file\n")
    # fname = "./data/" + fname
    with open("./data/" + fname, 'r') as f:
        vals = f.read()

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

    out = open("tests/" + fname[:-5]+".tab", "w")
    print >> out, new
=======
import sys
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
    args = sys.argv
    fname = args[1]
    if fname != "all":
        convert_file("./data/" + fname)
    
    else:
        os.chdir("./data")
        for file in os.listdir('.'):
            os.chdir("../")
            convert_file(file)
            os.chdir("./data")
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746


def main():
    fname = raw_input("enter name of file\n")
    fname = "data/" + fname
    with open(fname, 'r') as f:
        vals = f.read()
    vals = vals.split("\r\n")
    for i in range(len(vals)):
        vals[i] = vals[i].split(" ")
    new = ""
    temp = ""
    i = 0
<<<<<<< HEAD
    print vals
=======
    # print vals
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746
    vals = vals[:-1]
    while i < len(vals):
        frst = vals[i][0]
        cnt = 0
        temp = []
        while i < len(vals) and vals[i][0] == frst:
<<<<<<< HEAD
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
=======
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
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746
    print >> out, new


if __name__ == '__main__':
    data_to_tab()
<<<<<<< HEAD
    # main()
=======
    #main()
>>>>>>> f74c585f51c55b8207e31be5aa16e99652cfb746
