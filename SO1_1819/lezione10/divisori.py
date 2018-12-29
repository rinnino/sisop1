#! /usr/bin/env python3

# programma divisori.py
# calcola il numeri di divisori degli interi dati nel file di input

import sys

def main(infiles,outfile):
  g = open(outfile,"w");
  for infile in infiles:
    f = open(infile,"r");
    assert(f!=None and g!=None);
    for s in f:
      x = int(s)
      d = divisori(x)
      print(x,d,file=g)
    f.close();
  g.close();

def divisori( n ):
  assert(n>0)
  tot = 0
  for i in range ( 1,n+1 ):
    if n%i==0:
      tot += 1
  return tot


if len(sys.argv)<3:
  print("Uso:\n\t %s infile1 infile2 ... infile3 outfile" % sys.argv[0])
else:
  main(sys.argv[1:-1], sys.argv[-1] )
