/* Initialisation de la courbe E modulo 751 */
p=751
E = ellinit([0, 0, 0, -1, 727] * Mod(1, p));

/* Constante d'extension */
kappa = 20;

/* Fonction d'encodage de message */
encode(m, k) = {
  for(j=0, k-1,
    x = m*k + j;
    y_list = ellordinate(E, x);
    if(#y_list > 0,
      print("Point trouvé à j=", j, " : (", x, ", ", y_list[1], ")");
      return([x, y_list[1]])
    );
  );
}

/* Fonction de décodage de point */
decode(P) = P[1] \ kappa;


/*Exercice 4*/
/*1. Fonction de comptage du nombre de points d'une courbe naive en testant tous les point*/
nombrePointCourbeNaive() = {
    my(ElmtsCourbe = List(), bool);
    for(x = 0, p-1,
        for(y = 0, p-1,
            bool = ellisoncurve(E, [x, y]);
            if(bool == 1,
                listput(ElmtsCourbe, [x, y])
            );
        );
    );
    return(#ElmtsCourbe+1);
}
print("Le nombre de point de la courbe est ", nombrePointCourbeNaive());
print("Le nombre de point veritable est ", ellcard(E));