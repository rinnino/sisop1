#! /usr/bin/env python3
import random
import sys

def main(n,nomefile):
  f = open(nomefile,"w");
  tot = 0;
  for i in range( n ):
    x = random.randint(-100, 100)
    print(x,file=f)
    tot += x*x
  f.close();
  print("Somma dei quadrati:", tot);

if len(sys.argv)!=3:
  print("Uso:\n\t %s numero_interi file" % sys.argv[0])
else:
  main(int(sys.argv[1]), sys.argv[2] )
