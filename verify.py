def main():
    fname = raw_input("enter name of file\n")
    with open(fname, 'r') as f:
        vals = f.read()

    op = raw_input("trim the frequency from test file\n")

    gnd_trth = raw_input("enter name of ground truth\n")

    with open(gnd_trth, 'r') as f:
        gnd = f.read()

    vals = vals.split("\n")
    vset = set()
    for i in range(len(vals)):
        vals[i] = vals[i].split(" ")
        vals[i] = vals[i][1:]
        if op == 1:
            vals[i] = vals[i][:-1]
        vals[i] = map(int, vals[i])
        vals[i] = sorted(vals[i])
        vals[i] = map(str, vals[i])
        vals[i] = "".join(vals[i])
        vset.add(vals[i])

    # vals = sorted(vals)
    gnd = gnd.split("\n")
    gset = set()
    for i in range(len(gnd)):
        gnd[i] = gnd[i].split(" ")
        gnd[i] = gnd[i][:-1]
        gnd[i] = map(int, gnd[i])
        gnd[i] = sorted(gnd[i])
        gnd[i] = map(str, gnd[i])
        gnd[i] = "".join(gnd[i])
        gset.add(gnd[i])

    # gnd = sorted(gnd)
    if len(vals) != len(gnd):
        print "some patterns are missing!"

    pres = 0
    rec = 0

    for pttrn in vset:
        if(pttrn in gset):
            pres += 1

    for pttrn in gset:
        if(pttrn in vset):
            rec += 1

    pres = pres / float(len(gset))
    rec = rec / float(len(vset))

    print "precision = " + str(pres)
    print "recall = " + str(rec)

    if pres == 1.0 and rec == 1.0 and len(gnd) == len(vals):
        print "output is CORRECT"


if __name__ == '__main__':
    main()
