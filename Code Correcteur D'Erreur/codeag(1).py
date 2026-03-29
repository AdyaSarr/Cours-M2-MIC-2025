# Corps fini F_4
F.<w> = GF(4, modulus=x^2 + x + 1)

# Courbe elliptique y^2 + y = x^3
E = EllipticCurve(F, [0,0,1,0,0])

# Points rationnels affines
points = [P for P in E.points() if not P.is_zero()]
print("Points rationnels affines :", points)

# Anneau de polynômes
R.<x,y> = PolynomialRing(F, 2)
# diviseur = 3*P0 
#L(diviseur)
functions = [R(1), x, y]


G = Matrix(F, len(functions), len(points),
           lambda i,j: functions[i](points[j][0], points[j][1]))

print("Matrice génératrice :")
print(G)

# Code linéaire
C = LinearCode(G)
print("Paramètres du code :")
print("n =", C.length())
print("k =", C.dimension())
print("d =", C.minimum_distance())
