import sys

z = -0.302416
n = -0.1
f = -100.0

if len (sys.argv) > 1 :
    z = float(sys.argv[1]) 
    
if len (sys.argv) > 2 :
    n = float(sys.argv[2])

if len (sys.argv) > 3 :
    f = float(sys.argv[3])     





zMinusOneOne = (f+n)/(f-n) + (1/z)*((-2*f*n)/(f-n))
zZeroOne = 0.5*zMinusOneOne + 0.5

print "z view space = ", z
print "z [-1,1] = ", zMinusOneOne
print "z [0,1] = ", zZeroOne