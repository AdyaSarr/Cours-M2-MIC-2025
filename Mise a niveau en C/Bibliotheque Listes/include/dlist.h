#if !defined(DLIST_H)
#define DLIST_H
/*Autor: Adya SARR 03/02/2000*/

#include <stdio.h> //pour FILE
/*Comment la list sera generique donc on sait pas
    -comment copier un element
    -comment detruire un element
    -comment comparer deux elements
    -ou meme comment afficher pour deboguer
C'est pourquoi je vais fournir ces comprtement sous forme de fonctions que la librerie appellera: ce sont les les callback*/

/*contrat:
    -Entree: const void *element
    -pointeur alloué dynamiquement vers une copie de l’objet, ou NULL si échec mémoire.
    -Propriété : la lib possédera cette copie et l’enverra plus tard à dlist_free_fn*/
typedef void *(*dlist_copy_fn)(const void *element);

/*contrat:
    -Entrée : void *p (peut être NULL).
    -Effet : libérer + nettoyer l’objet pointé. Doit supporter d’être appelé même si l’objet est partiellement construit.*/
typedef void (*dlist_free_fn)(void *element);

/*contrat:
    -Retour : <0 si a<b, 0 si égalité, >0 si a>b.
    -Propriétés : ordre strict faible (transitif, cohérent) pour garantir un tri correct et unique fiable.*/
typedef int (*dlist_cmp_fn)(const void *element1, const void *element2);

/*sontrat:
    -Ne doit pas modifier l’objet. Doit écrire dans FILE *out*/
typedef void (*dlist_print_fn)(const void *element, FILE *out);

/*Creation d'une enumeration pour les codes d'erreurs*/
typedef enum{
    DLIST_OK =0,
    DLIST_NOT_FOUND =1,
    DLIST_NOMEM,
    DLIST_INVALID_ARG,
    DLIST_INDEX_OOB
}dlist_status;


/*definition de la structure des noeuds de la dliste noeud(data, prev, next)*/
typedef struct DListnode {
    void *data;//la donnée contenue dans le noeud
    struct DListnode *prev;//le noeud precedent
    struct DListnode *next;//le noeud suivant
}DListnode;


/*definition de la structure de la liste(head, tail, size + callback)*/
typedef struct DList{
    DListnode *head;//premier element de la liste
    DListnode *tail;//le dernier element de la liste
    size_t size; //le nombre d'element de la liste
    dlist_copy_fn copy;
    dlist_free_fn free_fn;
    dlist_cmp_fn cmp;
    dlist_print_fn print;
}DList;


/*=================================================Les constructeurs et les destructeurs==================================================*/


/*contrat:O(1)
    -Entrées : 4 callbacks
    -Sortie : un pointeur DList* nouvellement alloué (sur le tas) ou NULL si manque mémoire.
    -Effets :
        -Allouer l’objet DList avec calloc(1, sizeof *list)
        -Initialiser : head = tail = NULL, size = 0, stocker les callbacks.
    -Post-conditions (invariants) :
        -Liste vide : head == NULL, tail == NULL, size == 0.
        -Les callbacks sont conservés tels quels pour usage futur.
    -Erreurs :
        -Si allocation échoue → retourner NULL. Pas d’effet de bord.*/
DList *dlist_create(dlist_copy_fn copy, dlist_free_fn free_fn, dlist_cmp_fn cmp, dlist_print_fn print);

/*contrat:O(n)
    -Entrées : list
    -Effets :
        -Parcourir tous les nœuds de la liste.
        -Pour chaque nœud :
            -si free_fn != NULL et node->data != NULL → appeler free_fn(node->data).
        -free(node)
    -Post-conditions (invariants) :
        -La structure DList existe toujours.
        -Les callbacks restent inchangés.
    -Erreurs :
        -Pas de code d’erreur ici*/
void dlist_clear(DList *list);


/*contrat:O(n)
    -Entrées : list
    -Effets :
        -Appeler dlist_clear(list).
        -Puis free(list).
    -Post-conditions (invariants) :
        -L’objet DList est libéré. Le pointeur de l’appelant est désormais dangling → bonne pratique côté appelant : L = NULL; juste après l’appel.
    -Erreurs :
        -Pas de code d’erreur*/
void dlist_destroy(DList *list); //vide et detruit carrement la placement memoire occuper par la liste


/*=================================================Acces en O(1)==================================================*/
/*contrat:
    -But : retourner le nombre d’éléments.
    -NULL : retourne 0 si list == NULL.*/
size_t dlist_size(DList *list);

/*contrat:
    -But : vrai (1) si la liste est vide, faux (0) sinon.
    -Cas NULL : retourne 1 (considérée vide).*/
int dlist_empty(DList *list);


/*contrat:
    -But : obtenir le pointeur vers la donnée du premier nœud
    -Cas vide / NULL : retourne NULL si list == NULL ou liste vide.
    -Important :
        -Ne pas allouer / copier : tu renvoies le pointeur stocké.
        -Le pointeur retourné devient invalide si l’élément est supprimé/clear/free plus tard.*/
void *dlist_front(DList *list);


/*contrat:
    -But : comme front, mais pour le dernier nœud (tail->data)..
    -Cas vide / NULL : retourne NULL si vide ou list == NULL.*/
void *dlist_back(DList *list);

/*=================================================Insertion==================================================*/
/*Contrat commun
    -Complexité : O(1).
    -Erreurs : DLIST_INVALID_ARG si list==NULL ou (pour insert_*) elementUL).

    -Mise à jour : head, tail, size++, liens prev/next.
    -posi doit appartenir à la même liste.*/
dlist_status dlist_push_front(DList *list,const void *element);
dlist_status dlist_push_back(DList *list,const void *element);
dlist_status dlist_insert_before(DList *list,DListnode *posi, const void *element);
dlist_status dlist_insert_after(DList *list,DListnode *posi, const void *element);


/*=================================================Iteration==================================================*/
/*contrat:
    -Complexité : chaque fonction est O(1).
    -Robustesse :
        -dlist_begin/rbegin(NULL) → NULL.
        -dlist_next/prev(NULL) → NULL.
        -dlist_data(NULL) → NULL.
    -Sémantique :
        -begin renvoie le premier nœud (tête).
        -rbegin renvoie le dernier nœud (queue).
        -next/prev renvoient le voisin immédiat ou NULL si bord.
        -data renvoie le pointeur stocké dans le nœud (pas une copie).
    -Aucune modification de la liste dans ces fonctions.*/

DListnode *dlist_begin(const DList *list);//premier noeud (NULL si vide ou list==NULL)
DListnode *dlist_rbegin(const DList *list);//dernier noeud (NULL si vide ou list==NULL)
DListnode *dlist_next(const  DListnode *node);//nœud suivant (NULL si node==NULL ou fin)
DListnode *dlist_prev(const DListnode *node);//nœud précédent (NULL si node==NULL ou début)
void *dlist_data(const DListnode *node);// pointeur vers la donnée (NULL si node==NULL)


/*=================================================Supprimer==================================================*/

/*contrat:
    pop_front(list) / pop_back(list)
        -Effet : supprime le premier / le dernier nœud.
        -Données : si list->free_fn != NULL et node->data != NULL, appelle free_fn(node->data).
        -Nœud : toujours libéré (free(node)).
        -Mise à jour : réparer les liens, ajuster head/tail, décrémenter size.
        -Cas liste vide : retourner DLIST_INVALID_ARG.*/
dlist_status dlist_pop_front(DList *list);
dlist_status dlist_pop_back(DList *list);

/*contrat
    -Effet : supprime le nœud node, qu’il soit en tête, en queue ou au milieu.
    -Données : même règle que ci-dessus (appel à free_fn si défini).
    -Nœud : free(node).
    -Mise à jour : réparer liens voisins, corriger head/tail si besoin, size--.
    -Préconditions : list != NULL, node != NULL, et node appartient à list a prendre en compte.*/
dlist_status dlist_erase(DList *list, DListnode *node);

/*contrat:
    -Effet : retire node de la liste sans détruire data.
    -Retour : le pointeur data du nœud.
    -Nœud : libéré (free(node)).
    -Mise à jour : liens prev/next, head/tail, size--.
    -Important : ne pas appeler free_fn dans extract.
    -Préconditions : comme erase. Retourne NULL si invalide.*/
void *dlist_extract(DList *list, DListnode *node);


/*=================================================Recherche==================================================*/

/*Invariants / Effets de bord
    -Aucune de ces fonctions ne modifie la liste (ni les données).
    -Les nœuds retournés restent valides tant qu’on ne les supprime pas ailleurs.
    -Si il yas des éléments dont data==NULL, ta cmp ou ton pred doivent savoir gérer ce cas.*/


/*contrat:
    -But:Retourner le premier nœud dont la donnée est égale à key selon list->cmp.
    -Entrées : list (peut être NULL), key (peut être NULL si ta cmp sait gérer).
    -Sortie : pointeur vers le nœud trouvé, sinon NULL.
    -Ordre : toujours le premier en partant de head.
    -Comparaison : list->cmp(node->data, key) == 0.
    -Cas limites
        -Si list==NULL → NULL.
        -Si list->cmp==NULL → recommandé : retourne NULL*/
DListnode *dlist_find_first(const DList *list,const void *key);

/*contrat:
    -But:Trouver le premier nœud pour lequel un prédicat fourni par l’utilisateur renvoie vrai.
    -Entrées : pred(const void *elem, void *ctx) (doit retourner 0/1), ctx transmis tel quel.
    -Sortie : nœud trouvé, sinon NULL.
    -Comportement : pred ne doit pas modifier la structure de la liste (pas d’erase/insert dedans).
    -Robustesse : si pred==NULL → NULL.*/
DListnode *dlist_find_if(const DList *list, int (*pred)(const void *elem, void *ctx), void *ctx);

/*contrat:
    -But: Compter le nombre d’occurrences de key selon list->cmp.
    -Entrées : list, key.
    -Sortie : un size_t (0 si rien / cas invalides).
    -Comparaison : même règle que find_first.
    -Cas limites
        -list==NULL → 0.
        -cmp==NULL → recommandé : 0*/
size_t dlist_count(const DList *list, const void *key);




/*=================================================Haut Niveau==================================================*/

/*contrat:
    But : inverser l’ordre des nœuds sans allouer.
    -Complexité : O(n) (on parcourt tous les nœuds).
    -list==NULL → no-op.
    -Échanger prev/next dans chaque nœud, puis permuter head/tail.
    -Invariants après :
        -head->prev==NULL, tail->next==NULL (si non vide).
        -size inchangé.*/
void dlist_reverse(DList *list);

/*contrat:
    -But : chaîner src à la fin de dst sans copier.
    -Complexité : O(1) (juste des pointeurs & tailles).
    -dst==NULL ou src==NULL → no-op.
    -Si src vide → no-op.
    -Si dst vide → dst = src (tête/queue/size copiés), puis vider src.
    -Sinon, relier dst->tail <-> src->head, mettre à jour dst->tail, dst->size += src->size, puis vider src (head=tail=NULL, size=0).
    -Interdit : dst==src (auto-concat) → documente erreur/no-op.
    -Invariants :
        -dst cohérente ; src devient vide.*/
dlist_status dlist_concat(DList *dst, DList *src);                  

/*contrat:
    -But : insérer en bloc tous les nœuds de src avant le nœud pos de dst.
    -Complexité : O(1) (raboutage).
    -dst==NULL ou src==NULL → no-op.
    -src vide → no-op.
    -pos==NULL → par design, choisis :
        -option A : équivaut à dlist_concat(dst, src) (j'ai choisi celui-la);
        -option B : DLIST_INVALID_ARG.
    -pos doit appartenir à dst (documente ; en debug : assert).
    -Interdit : dst==src.
    -Raboutage :
        -L = pos->prev, relier L <-> src->head et src->tail <-> pos.
        -Ajuster dst->head si insertion en tête.
        -dst->size += src->size, puis vider src.
    -Invariants :
        -dst cohérente ; src vide à la fin.*/
dlist_status dlist_splice_before(DList *dst, DListnode *pos, DList *src);


/*contrat:
    -But : séparer src en deux listes :
    -src garde de head jusqu’à pos (inclus),
    -la nouvelle liste retournée contient pos->next .. tail.
    -Complexité : O(k) où k = nombre de nœuds déplacés (pour recalculer la taille).
    -src==NULL → retourne NULL ou nouvelle liste vide (choix A/B ; recommande A = retourner NULL).
    -Toujours retourner une nouvelle liste créée avec les mêmes callbacks que src.
    -Cas pos :
        -pos==NULL → tout passe à la nouvelle liste, src devient vide.
        -pos==tail → nouvelle liste vide.
        -Sinon → détacher R = pos->next, pos->next=NULL, R->prev=NULL, et la nouvelle liste devient [R .. ancien_tail].
    -Tailles :
        -Recalcule out->size en comptant ses nœuds (O(k)), puis src->size -= out->size.
    -Invariants :
        -Les deux listes sont cohérentes (head/tail, bords prev/next).
        -Les callbacks des deux listes sont identiques.*/
DList *dlist_split_after(DList *src, DListnode *pos);              


/*=================================================Algorithme==================================================*/

/* -Complexités : dlist_unique = O(n), dlist_remove_if = O(n), dlist_sort = O(n log n).
   -Pré-requis : sort/unique exigent list->cmp != NULL (sinon DLIST_INVALID_ARG).
   -Effets mémoire : les nœuds supprimés sont libérés (free(node)), et leurs données aussi si free_fn est non-NULL.*/


/*contrat:
    -But:Ne conserver qu’un représentant par run d’éléments égaux (adjacents) selon cmp.
    -Entrée : list (peut être vide).
    -Retour : DLIST_OK ou DLIST_INVALID_ARG si list==NULL ou cmp==NULL.
    -Comportement :
        -Parcours avant : pour chaque paire (p, nxt), si cmp(p->data, nxt->data) == 0, supprimer nxt.
        -Ne touche pas à des doublons non-adjacents. (Pour tout dédoublonner, faire sort puis unique.)
    -Cas limites:
        -Liste vide/1 élément → no-op.
        -Données NULL : le cmp les gére*/
dlist_status dlist_unique(DList *list);


/*contrat:
    -But:Supprimer tous les nœuds pour lesquels pred(elem, ctx) est vrai.
    -Entrées : pred (obligatoire), ctx transmis tel quel.
    -Retour : DLIST_OK ou DLIST_INVALID_ARG si list==NULL ou pred==NULL.
    -Comportement :
        -Parcours sûr : sauver next = node->next avant de supprimer.
        -Sur suppression : appeler free_fn(data) si défini, puis free(node).
    -Cas limites:
        -Liste vide → OK.
        -pred qui ne matche rien → liste inchangée.
        -pred qui matche tout → liste vide à la fin.
        -data==NULL → pred doit l’accepter (ou documenter).*/
dlist_status dlist_remove_if(DList *list, int (*pred)(const void *elem, void *ctx), void *ctx);

/*contrat:
    -But:Trier la liste in-place avec stabilité (les éléments égaux conservent l’ordre initial).
    -Entrée : list non-NULL, cmp non-NULL.
    -Retour : DLIST_OK ou DLIST_INVALID_ARG.
    -Stratégie imposée : mergesort (liste)
        -split_middle(head) : couper la liste en deux moitiés à l’aide d’un duo slow/fast :
            -slow avance de 1, fast de 2 ; quand fast atteint la fin, couper avant slow.
            -prev_slow->next = NULL pour séparer les moitiés ; veiller à remettre les prev à NULL au début de chaque sous-liste.
            -Et retourne a la fin la tete de la liste a droite
        -msort_rec(head) : cas de base 0/1 élément ; sinon, trie récursivement L et R, puis fusionne.
        -merge_sorted(a,b,cmp) : fusion stable :
            -Choisir a quand cmp(a->data, b->data) <= 0 (le <= garantit la stabilité).
            -Relier les prev/next proprement à chaque rattachement.*/
dlist_status dlist_sort(DList *list);


/*=================================================Fonctionnel==================================================*/
/*contrat:
    -But : Appliquer map_fn à chaque elem de in pour produire un nouvel élément et renvoyer une nouvelle liste contenant ces résultats.
    -Entrées : in (peut être vide), map_fn (obligatoire), ctx arbitraire, out_free pour libérer les éléments produits en cas d’échec ou à la destruction.
    -Sortie : une nouvelle DList* (callbacks internes : copy==NULL, free_fn==out_free, cmp==in->cmp si pertinent, print==in->print au choix), ou NULL si OOM.
    -Comportement :
        -Pour chaque nœud e de in, calculer y = map_fn(e->data, ctx).
        -Insérer y dans la sortie sans recopier (copy==NULL) car map_fn est censé retourner déjà une nouvelle donnée (sur le tas).
    -Erreurs :
        -Si map_fn==NULL → NULL.
        -Si une insertion échoue → détruire proprement la liste de sortie (en appelant out_free sur tout ce qui a déjà été ajouté) et retourner NULL.
    -Cas limites
        -in==NULL → retourne NULL (ou liste vide — choisis et documente).
        -map_fn qui retourne NULL pour un élément → considère que c’est un échec global (OOM) ou skip l’élément : choisis une politique et documente-la (recommandé : échec global).*/
DList *dlist_map(const DList *in,
                 void *(*map_fn)(const void *elem, void *ctx),
                 void *ctx,
                 dlist_free_fn out_free);

/*contrat:
    -But : Garder seulement les éléments pour lesquels pred(elem, ctx) est vrai, dans une nouvelle liste.
    -Entrées : pred (obligatoire).
    -Sortie : nouvelle DList* avec la même politique mémoire que in (copy/free/cmp/print copiés tels quels).
    -Comportement :
        -Pour chaque elem qui passe pred :
            -Si in->copy != NULL → insérer une copie (profonde) via copy(elem).
            -Sinon (copy==NULL) → insérer le pointeur tel quel (aliasing).
    -Erreurs :
        -pred==NULL → NULL.
        -Échec de copy/malloc → détruire proprement la sortie et retourner NULL.
    -Cas limites
        -Si copy==NULL et free_fn!=NULL, double-libération potentielle si tu filtres vers une nouvelle liste qui croit posséder les données.
→ Recommandation : dans ta doc globale, exige free_fn==NULL quand copy==NULL (mode “pas de propriété”). Sinon, refuse la création de la sortie ou clone en forçant une copie (décide et documente).*/
DList *dlist_filter(const DList *in,
                    int (*pred)(const void *elem, void *ctx),
                    void *ctx);

/*contrat:
    -But : Copier structure + données selon la politique mémoire de in.
    -Sortie : nouvelle DList* avec mêmes callbacks.
    -Comportement :
        -Si in->copy != NULL → copie profonde de chaque élément.
        -Sinon → copie superficielle (alias des pointeurs) ; documente fortement que ni in ni le clone ne doivent tenter de free ces pointeurs (donc free_fn devrait être NULL si copy==NULL).
    -Erreurs : OOM → détruire proprement la sortie partielle, retourner NULL.
    -Tests
        -Clone d’int avec copy → adresses distinctes ; modifier le clone n’affecte pas l’original.
        -Clone de char* avec copy=strdup → profonde.
        -Clone quand copy==NULL → alias ; vérifier qu’un clear du clone ne détruit pas les données (si free_fn==NULL).*/
DList *dlist_clone(const DList *in);

/*contrat:
    -But : Exporter les pointeurs data tels quels dans un tableau void**.
    -Entrées : list, out_len (peut être NULL).
    -Sortie : void** arr de longueur list->size (ou NULL si OOM).
    -Comportement :
        -Remplir arr[i] dans l’ordre de la liste.
        -Si out_len non-NULL, écrire la longueur.
    -Mémoire : l’appelant doit free(arr) ; les pointeurs contenus restent la propriété de la liste (sauf si tu les extract ensuite).
    -Cas limites
        -list==NULL → retourne NULL (ou tableau vide : choisis et documente).
        -size==0 → arr peut être NULL ou un bloc 0 → choisis (recommandé : pointer non-NULL de taille 0 pas nécessaire ; NULL acceptable si tu documentes).*/
void **dlist_to_array(const DList *list, size_t *out_len);


/*contrat:
    -But: Construire une nouvelle liste à partir d’un tableau de pointeurs.
    -Entrées : arr (peut être NULL si len==0), len, callbacks.
    -Sortie : nouvelle DList* ou NULL si OOM.
    -Comportement :
        -Pour i=0..len-1 :
        -Si copy!=NULL → insérer une copie de arr[i].
        -Sinon → insérer arr[i] tel quel.
    -Erreurs : échec d’insertion → détruire proprement et NULL.
    -Cas limites
        -len==0 → liste vide.
        -Cohérence copy/free : même recommandation que plus haut (si copy==NULL, privilégier free==NULL).*/
DList *dlist_from_array(void *const *arr, size_t len,
                        dlist_copy_fn copy, dlist_free_fn free_fn,
                        dlist_cmp_fn cmp, dlist_print_fn print);

#endif // DLIST_H
