#! /usr/bin/env python3

# funzione che visualizza degli interi dispari con un numero massimo
# di cifre assegnato da linea di comandi
# usate la ridirezione per salvare gli interi in un file in formato testo
 
import random
import sys

def main(n,cifre):
  for i in range(n):
    x = random.randint(0, 10**(cifre))
    if x%2==0:
      x += 1
    while x%3==0 or x%5 ==0 or x%7==0:
      x += 2
    print(x)


if len(sys.argv)!=3:
  print("Uso:\n\t %s numero_interi cifre" % sys.argv[0])
else:
  main(int(sys.argv[1]), int(sys.argv[2]) )
  
  
      
