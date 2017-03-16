import os


def main():
    fname = raw_input("enter name of file\n")
    with open(fname, 'r') as f:
        vals = f.read()

    gnd_trth = raw_input("enter name of ground truth\n")

    dirs = os.listdir('.')
    dirs = [d for d in dirs if os.path.isdir(d)]
    for d in dirs:
        os.chdir("./" + d)
        if gnd_trth in os.listdir('.'):
            with open(gnd_trth, 'r') as f:
                gnd = f.read()
            break
        os.chdir("../")

    vals = vals.split("\n")
    vset = set()

    for i in range(len(vals)):
        vals[i] = vals[i].split(" ")

        if i == 0:
            if len(vals[0][1:]) == vals[0][0]:
                op = 0
            else:
                op = 1
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
    print "len(vals) = " + str(len(vals)) + " len(gnd) = " + str(len(gnd))

    pres = 0
    rec = 0

    for pttrn in vset:
        if pttrn in gset:
            pres += 1

    for pttrn in gset:
        if pttrn in vset:
            rec += 1

    pres = pres / float(len(vset))
    rec = rec / float(len(gset))

    print "precision = " + str(pres)
    print "recall = " + str(rec)

    if pres == 1.0 and rec == 1.0 and len(gnd) == len(vals):
        print "output is CORRECT"


if __name__ == '__main__':
    main()
