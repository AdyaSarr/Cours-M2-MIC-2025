#if !defined(FP_H)
#define FP_H



typedef struct
{
    int p;
}Fpctx;

typedef struct 
{
    int n;//taille de la memoire a allouer pour le polynome pour eviter de faire tjrs des realloc
    int deg;//degre du polynome egal a -1 si le polynome est nul
    int *coefficient;//les coefficients du polynome
    Fpctx *k;//
}Poly;


/*=============================================Construction de polynome===================================================*/
Poly *poly_creat(Fpctx *p, int n);
void poly_free(Poly *p_de_x);

/*=============================================Operation sur les polynomes================================================*/
Poly *poly_multiplication(Poly *p_de_x, Poly *q_de_x);//calcul la multiplica
#endif // FP_H
