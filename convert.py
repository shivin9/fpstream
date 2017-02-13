# this version is to create files which fpgrowth can take

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
    vals = vals[:-1]
    while i < len(vals):
        frst = vals[i][0]
        cnt = 0
        temp = []
        while i < len(vals) and vals[i][0] == frst:
            cnt += 1
            temp.append(vals[i][1])
            #temp = temp + vals[i][1] + ","
            i += 1
        #temp = str(cnt) + " " + temp + "\n"
        temp = list(set(temp))
        temp = " ".join(temp)
        temp = temp + "\n"
        new += temp

    out = open(fname+".tab", "w")
    print >> out, new

if __name__ == '__main__':
    main()
