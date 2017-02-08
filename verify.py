def main():
    fname = raw_input("enter name of file\n")
    with open(fname, 'r') as f:
        vals = f.read()

    op = raw_input("trim the frequency from test file\n")

    gnd_trth = raw_input("enter name of ground truth\n")

    with open(gnd_trth, 'r') as f:
        gnd = f.read()

    vals = vals.split("\n")
    for i in range(len(vals)):
        vals[i] = vals[i].split(" ")
        vals[i] = vals[i][1:]
        if op == 1:
            vals[i] = vals[i][:-1]
        vals[i] = map(int, vals[i])
        vals[i] = sorted(vals[i])
    vals = sorted(vals)

    gnd = gnd.split("\n")
    for i in range(len(gnd)):
        gnd[i] = gnd[i].split(" ")
        gnd[i] = gnd[i][:-1]
        gnd[i] = map(int, gnd[i])
        gnd[i] = sorted(gnd[i])
    gnd = sorted(gnd)

    if len(vals) != len(gnd):
        print "some patterns are missing!"

    flag = 0
    coll = []

    while i < len(vals):
        if(vals[i] == gnd[i]):
            i += 1
        else:
            j = i
            while vals[i] != gnd[j]:
                flag += 1
                coll.append(gnd[j])
                j += 1
            i = j

    if flag == 0 and len(gnd) == len(vals):
        print "output is CORRECT"
    elif flag == 0:
        print "result is short of " + str(len(gnd) - len(vals)) + " transactions"

if __name__ == '__main__':
    main()
