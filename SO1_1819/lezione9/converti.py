#!/usr/bin/env python3
import sys

def main(infile,outfile):
  with open(infile,"r") as f:
    with open(outfile,"w") as g:
      for s in f:
        r = ""
        for c in s:
          if c.isupper(): c = c.lower()
          elif c.islower(): c = c.upper()
          r +=c
        g.write(r)
  return

if len(sys.argv)!=3:
  print("Uso:\n\t %s infile outfile" % sys.argv[0])
else:
  main(sys.argv[1], sys.argv[2])
