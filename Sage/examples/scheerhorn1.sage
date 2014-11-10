from sage.all import *
import sys
import itertools

load("./algorithmen.spyx")


def getDegOver(x,F):
    q = F.order()
    for i in itertools.count(1):
        if x**(q**i) == x:
            return i

###
q = 13
n = 7   # nu(n) | q+1
#n = 9  # nu(n) | q-1
#n = 4   # nu(n) | q+1
###

###
#q = 2
#n = 27 # nu(n) | q+1
###


###
#q = 4
#n = 9 # nu(n) | q-1
###



F = GF(q, 'a');
Fx = PolynomialRing(F,'x');
s = ordn(squarefree(n),q)


print "----------------------------------------"
print "----- Preliminaries"
print "F = GF("+str(q)+")\tn = "+str(n)
print "we examine the extension: "
print "E = GF(q^n) = GF(q^"+str(n)+") = GF("+str(q**n)+")"
print "K = GF(q^s) = GF("+str(q)+"^"+str(s)+") = GF("+str(q**2)+")",\
        "were s = ord_nu(n)(q) = ",s
print "H = GF(q^(sn)) = GF(q^("+str(s)+"*"+str(n)+")) = "+\
        "GF("+str(q**(2*n))+") -> total splitting field"

print "----------------------------------------"
print "----- Implementation details"

print "find irreducible Dickson-Polynom"
(a,b,f) = gen_irred_dickson(n,Fx)
print "\t","D_n(X,a)-b = D_"+str(n)+"(X,"+str(a)+") - "+str(b)+" is irreducible="\
        +str(f.is_irreducible())
print "\t", "a = ", a, "b = ",b

print "find extension of degree s="+str(s)+" named K"
g = Fx.gen()**2 - b*Fx.gen() + a**n;

if s > 1:
    print "\t","generated by root beta of ", g, "irred = ",g.is_irreducible()
    K = F.extension(g,'beta');
    beta = K.gen();
    print "K = ",K

    Gx = PolynomialRing(K, 'x');
else:
    print "\t", "consider g = X^2 - bX + a^n = ",g, "=",g.factor()
    K = F
    Gx = Fx
    beta = g.roots()[0][0];
    print "K = ", K
    print "\t", "with beta = ", beta


print "find extension of degree s*n="+str(s)+"*"+str(n)+"="+str(s*n)+" named H"
h = Gx('x')**n - beta;

print "\t", "generated by root th of ", h, "irred=", h.is_irreducible();

H = K.extension(h, 'th');
print "H = ",H

th = H.gen();
Hx = PolynomialRing(H, 'x');

print "----------------------------------------"
print "now th + a*th^(-1) should be root of D_n(X,a)"
print "\t", "D_"+str(n)+"("+str(th+a*th**(-1))+", "+str(a)+") = ",\
        f(th+a*th**(-1))


# Splitting of cyclotomic Polynomials
print "----------------------------------------"
print "----- Splitting of Cyclotomic Polynomials"
for i in divisors(n)+map(lambda d: d*n,divisors(s)[1:]):
    phi = Fx.cyclotomic_polynomial(i).factor();
    print "Phi_"+i.str()+" = ", phi, "over F"
    sys.stdout.write("\t")
    for (d,e) in phi:
        sys.stdout.write("[ "+str(Gx(d).factor())+" ] ")
    sys.stdout.write(" over K\n")

print "----------------------------------------"
print "Ord_q^2(th) = ",tau_order(th,K)
print "it should be: Ord_q(th) = f(x^s) for f=MiPo_F( Ord_q^s(th) )"
print "\t we have:\tf = ", mipo(q,s,tau_order(th,K))
print "\t and \t Ord_q(th) = ", tau_order(th,F)


print "----------------------------------------"
print "----- test tau-orders: "
#print "Ord_q(th^i):"
#for i in range(0,n+1):
    #print "i="+str(i)+" =>", tau_order(th**i, F),\
        #"\t\ti=-"+str(i)+" =>", tau_order(th**(-i), F)

#print "Ord_q^s(th^i):"
#for i in range(0,n+1):
    #print "i="+str(i)+" =>", tau_order(th**i, K),\
        #"\t\ti=-"+str(i)+" =>", tau_order(th**(-i), K)

print "Ord_q(th^i+a^i*th^(-i)) = Ord_q(D_i(th+a*th^-1) = "
for i in range(0,n+1):
    print "i="+str(i)+" =>", tau_order(th**i+a**i*th**(-i), F), "\t"\
            , " has deg over F: ", getDegOver(th**i+a**i*th**(-i),F)

#print "Ord_q^s(th^i+a^i*th^(-i)) = Ord_q^s(D_i(th+a*th^-1) = "
#for i in range(0,n+1):
    #print "i="+str(i)+" =>", tau_order(th**i+a**i*th**(-i), K)


print "----------------------------------------"
print "----- construct normal element"
cos = map(lambda c: c[0], cosets(n,q))
for l in cos:
    print "l=",l," Ord_q(th^l+a^l*th^-l) = ",\
            tau_order(th**l+a**l*th**(-l),F),"\t", \
            "Ord_q(th^l) = ", tau_order(th**l,F),"\t", \
            "Ord_q(a^l*th^-l) = ", tau_order(a**l*th**(-l),F)
print "Sum l in R_q(n) D_l(th+a*th^-1) has q-order:", \
        tau_order(sum(map(lambda l: th**l+a**l*th**(-l), cos)),F)


print "----------------------------------------"
print "----- cosets n=",n," q=",q
print_cosets(n,q)

