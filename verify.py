#!/usr/bin/python
import sys
import os


def main():
    args = sys.argv
    fname = args[1]
    print fname
    with open(fname, 'r') as f:
        vals = f.read()


    gnd_trth = args[2]
    missing = open(gnd_trth[:-4] + "_missing.ignore", 'w')

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
    vset = dict()

    op = 0
    cnt = 0

    # reading the test file
    for i in range(len(vals) - 1):
        vals[i] = vals[i].split(" ")

        if i == 0:
            if len(vals[0][1:]) == int(vals[0][0]):
                op = 0
            else:
                op = 1

        vals[i] = vals[i][1:]

        if op == 1:
            cnt = str(vals[i][-1])
            vals[i] = vals[i][:-1]
        vals[i] = map(int, vals[i])
        vals[i] = sorted(vals[i])
        vals[i] = map(str, vals[i])
        vals[i] = ",".join(vals[i])
        if op == 1:
            vset[vals[i]] = cnt
        else:
            vset[vals[i]] = '0'

    # vals = sorted(vals)
    gnd = gnd.split("\n")
    gset = dict()
    for i in range(len(gnd) - 1):
        gnd[i] = gnd[i].split(" ")

        if op == 1:
            cnt = str(gnd[i][-1][1:-1])

        gnd[i] = gnd[i][:-1]
        gnd[i] = map(int, gnd[i])
        gnd[i] = sorted(gnd[i])
        gnd[i] = map(str, gnd[i])
        gnd[i] = ",".join(gnd[i])

        if op == 1:
            gset[gnd[i]] = cnt
        else:
            gset[gnd[i]] = '0'

    # gnd = sorted(gnd)
    print "len(vals) = " + str(len(vals)) + " len(gnd) = " + str(len(gnd))

    pres = 0
    rec = 0
    wrng_cnt = 0
    flag = 0

    for pttrn in vset:
        if pttrn in gset:
            pres += 1
            if vset[pttrn] != gset[pttrn]:
                wrng_cnt += 1

    # wrng_cnt = 0

    for pttrn in gset:
        if pttrn in vset:
            rec += 1
        else:
            flag+=1
            missing.write(str(pttrn) + '\n')
            # print str(pttrn)

    pres = pres / float(len(vset))
    rec = rec / float(len(gset))

    print "precision = " + str(pres)
    print "recall = " + str(rec)
    print "wrong count = " + str(wrng_cnt)

    if pres == 1.0 and rec == 1.0 and len(gnd) == len(vals):
        print "output is CORRECT"

    if flag > 0:
        missing.flush()
        missing.close()
    if "./" + gnd_trth[:-4] + "_missing.ignore" in os.listdir('.'):
        os.remove("./" + gnd_trth[:-4] + "_missing.ignore")


if __name__ == '__main__':
    main()
