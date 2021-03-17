# TP2: Thread, synchronisation et ordonnancement

L'objectif principal de ce TP est de vous familiariser avec les concepts de synchronisation, d'interblocage et
d'ordonnancement.

Vous aurez deux tâches principales pour ce TP. La première consiste à implémenter une structure de données synchronisée,
soit une *blocking queue*. À l'aide de cette blocking queue, vous simulerez l'assignation de tâches exécutables sur un
processeur qui consistera d'un ensemble de threads.

## Tâche 1: implémentation d'une `blocking queue`

Une blocking queue est une queue qui peut bloquer lorsqu'on demande un élément (ou plusieurs) mais que celle-ci est
vide. Pour ceux qui ne savent pas comment fonctionne une queue, c'est une structure de donnée très simple, analogue à
une file d'attente dans vraie vie.

```
Élément 1 : Élément 2 : ... : Élément N
^                             |
Premier élément à servir      |
                              ^
                              Dernier élément à servir
```

Chaque élément s'ajoute à la fin de la queue (de la file) et le premier élément à en sortir est celui qui y est depuis *
le plus longtemps*. Une façon populaire d'implémenter une queue, et celle qui sera imposée ici, est d'utiliser une liste
chaînée. Une liste chaînée est simplement une structure qui contient un pointeur vers le prochain élément.

```
+-+-+-+-+-+-+            ---------->  +-+-+-+-+-+-+
- Élément 1 -            +            - Élément 2 -
+-----------+            -            +-----------+
- Pointeur --------------+            -Pointeur...-
+-+-+-+-+-+-+                         +-+-+-+-+-+-+
```

Le code à implémenter se trouve dans le fichier `blocking_queue.c`. Le fichier `blocking_queue.h`
contient les entêtes ainsi que la définition de la liste chaînée:

```C
typedef struct blocking_q_node_struct blocking_q_node;

struct blocking_q_node_struct {
task_ptr data;
blocking_q_node *next;
};

typedef struct {
pthread_mutex_t lock;
pthread_cond_t cond;
blocking_q_node *first;
size_t sz;
} blocking_q;
```

Vous devriez reconnaître le type de tous les champs utilisés dans la définition, sauf peut-être `blocking_q_node` qui
est le type du premier noeud. À l'intérieur d'un champ se trouve le prochain noeud et un pointeur vers la tâche à
exécuter (ne vous préoccupez pas de ce qu'est une tâche pour cette partie).

Comme vous le verrez dans un instant, vous serez responsable d'implémenter l'entièreté de cette structure de données. En
tant que concepteur de structures de données (un titre à ne pas prendre à la légère) vous êtes donc responsable de vous
assurer que chaque fonction préserve la structure dans un état cohérent. Cela veut dire que chaque opération préserve le
bon fonctionement de la structure pour des appels futurs et que l'information dans la structure reflète bien l'état de
celle-ci. La fonction à implémenter à l'exercice 1.1 est `static` et donc interne au fichier `blocking_queue.c`. Cela
veut dire qu'elle est probablement uniquement utile dans le contexte de l'implémentation interne de la structure 😉.
L'évaluation du code vérifie de la consistence de la structure.

### Exercice 1.1

```C
/**
 * Internal function to blocking_q. Takes an element
 * in the queue. This functions assumes the following
 * preconditions:
 *  - The thread has safe access to the queue
 *  - The queue is NOT empty
 * Also update the size.
 * @param q the queue
 * @return an element
 */
task_ptr __blocking_q_take(blocking_q *q);
```

Vous devez implémenter la fonction `___blocking_q_take`. Cette fonction assume que l'accès aux données de la queue est
possible de façon sécuritaire et que celle-ci n'est pas vide. Cette fonction retourne donc le premier élément de la
queue et met à jour la taille de celle-ci enregistrée dans la variable `sz`.

### Exercice 1.2

```C
/**
 * Create a blocking queue. Initializes the synchronisation primitives
 * and
 * @param q the queue
 * @return if init was successful.
 */
bool blocking_q_init(blocking_q *q);
```

Vous devez implémenter la fonction `blocking_q_init`. Cette fonction initialise les valeurs de la queue qui doivent être
initialisée et alloue les données qui doivent être allouée. Si une erreur se produit pendant l'allocation, vous devez
retourner faux.

### Exercice 1.3

```C
/**
 * Destroy a blocking queue. Removes the allocations of the data
 * and destroys the sync. primitives.
 * @param q ptr to the blocking queue
 */
void blocking_q_destroy(blocking_q *q);
```

Vous devez implémenter la fonction `blocking_queue_destroy`. Celle-ci nettoie la blocking queue en désallouant la
mémoire allouée ainsi qu'en détruisant les primitives de synchronisation. Elle doit libérer chaque noeuds, mais pas le
contenu des noeuds. De plus, les procédures `_destroy` ne libèrent pas le pointeur reçu en paramètre. La raison est
que la structure pourrait être allouée sur le stack ou sur le heap et la fonction doit être correcte indépendement de
comment l'allocation à été faite.

### Exercice 1.4

```C
/**
 * Put a task in the blocking queue. This task can fail if no
 * memory is available to allocate a new entry in the queue
 * @param q the queue
 * @param data the data description to put inside the queue
 * @returns if the data was put correctly inside the queue.
 */
bool blocking_q_put(blocking_q *q, task_ptr data);
```

Vous devez implémenter la fonction `blocking_q_put`. Cette fonction ajoute un nouveau noeuds dans la liste chaînée. Comme
il est possible que des threads attendent l'ajout d'un nouvel élément dans la liste, il faut aussi signaler qu'un nouvel
élément à été ajouté. De plus, la taille de la liste doit être mise à jour (puisqu'un élément à été ajouté).

### Exercice 1.5

```C
/**
 * Get an element in the blocking queue. If the queue is empty,
 * the current thread is put to sleep until an element is added
 * to the queue.
 * @param q the blocking queue
 * @return the element
 */
task_ptr blocking_q_get(blocking_q *q);

```

Vous devez implémenter la fonction `blocking_q_get`. Cette fonction va chercher le premier élément dans la liste. Si
aucun élément ne se trouve dans la liste, le thread doit être endormi, et ce réveiller lors de l'ajout d'un nouvel
élément dans la queue (pour pouvoir le retourner). Cette fonction ne doit donc jamais null. Suite à l'appel de cette
fonction, la taille de la liste doit être mise à jour.

### Exercice 1.6

```C
/**
 * Drain as many elements as possible into the area allowed
 * by the pointer. This function does not block.
 * @param q the queue
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @return the number of entries written.
 */
size_t blocking_q_drain(blocking_q *q, task_ptr *data, size_t sz);
```

Vous devez implémenter la fonction `blocking_q_drain`. Cette fonction prend le maximum d'élément possible dans la
queue (*ce maximum peut être 0*). Cette fonction ne bloque jamais. Elle retourne le nombre d'élément qui ont été placés
dans la liste reçue en argument. Attention à ne pas dépasser la longueur de la liste. Le pointeur `data` sert de tableau
récepteur aux données de la queue.

### Exercice 1.7

```C
/**
 * Drain at least min elements in the buffer. This function
 * might block if there are not enough elements to drain.
 * @param q the queue
 * @param data the pointer where to store the data
 * @param sz the maximum area available in the buffer
 * @param min the minimum amounts of elements to drain (must be less than sz)
 * @return the number of elements written
 */
size_t blocking_q_drain_at_least(blocking_q *q, task_ptr *data, size_t sz, size_t min);
```

Vous devez implémenter la fonction `blocking_q_drain_at_least`. Cette fonction retourne le maximum d'éléments possible,
tout en s'assurant d'en retourner au moins `min`. Cette fonction peut bloquer, puisqu'il n'y as pas nécessairement `min`
éléments dans la blocking queue au moment de l'appel.

### Exercice 1.8

```C
/**
* Check the first element in the queue. This will allocate storage for a copy
* of the character. If the allocation fails, this function returns false.
* @param q the queue
* @param c pointer to a pointer where an allocated char will be stored
* @return if there is an element allocated in the pointer
*/
bool blocking_q_peek(blocking_q *q, task **c);
```

Vous devez implémenter la fonction `blocking_q_peek`. Cette fonction observe le premier élément de la liste (s'il
existe). La fonction retourne s'il existe un tel élément. Si c'est le cas, une case mémoire est allouée et le pointeur
de la tâche est affectée vers cette case mémoire. Chaque utilisateur de la fonction est donc responsable de désallouer
la mémoire allouée par
`blocking_q_peek`.

## Tâche 2

Maintenant que vous avez implémenté l'entièreté d'une blocking queue, vous êtes prêt à passer aux choses sérieuses. Vous
devez maintenant mettre le chapeau du *scheduler* et d'affecter des tâches au processeur. Comme vous n'avez pas accès au
processeur de votre ordinateur directement, un certain nombre de *threads* seront utilisées pour simuler un processeur.

À partir de maintenant, on travaille dans `main.c`.

Les processeurs seront contenus dans les structures suivantes:

```C
typedef struct {
int id;
long sched_t;
long work_t;
long real_t;
long wait_t;
pthread_mutex_t lock;
blocking_q *tasks;
} processor;
```

Le processeur contient une mutex pour protéger son information, ainsi qu'une blocking queue qui contiendra les tâches
planifiées à ce processeur. Lorsqu'une tâche est terminée, le processeur passe à la suivante. Le champ ID contient le
numéro du processeur. Les variables `sched_t`,`work_t`, `real_t`, `wait_t` doivent contenir, respectivement, le temps de
travail planifié à ce processeur, le temps de travail effectué (théorique), le temps de travail effectué (réel) et le
temps d'attente. L'unité est la milliseconde pour ces quatres champs.

### Exercice 2.1

```C
/**
 * Initialises a processor structure. This can fail if there is no
 * memory for a tasks list, it's initialisation fails or the mutex
 * cannot be created.
 * @param id the ID of the processor
 * @param p the processor
 * @return if the initialization was successful
 */
bool processor_init(int id, processor *p);
```

Vous devez implémenter la fonction `processor_init`, qui alloue les ressources qui vont être nécessaire à l'exécution d'
un processeur et configure la structure du processeur correctement.

### Exercice 2.2

```C
/**
 * Destroy a processor structure
 * @param p ptr to the structure
 */
void processor_destroy(processor *p);
```

Vous devez implémenter la fonction `processor_destroy` qui désalloue les ressources utilisées par le processeur. Encore une fois,
les procédures `_destroy` ne libèrent pas le pointeur reçu en paramètre. La raison est que la structure pourrait être allouée sur
le stack ou sur le heap et la fonction doit être correcte indépendement de comment l'allocation à été faite.


### Exercice 2.3

```C
void *processor_run(void *v_self);
```

Vous devez implémenter la fonction `processor_run`. Cette fonction s'occupe d'exécuter une tâche qu'elle reçoit. Les 4
types de tâches (`t->type`) sont respectivement 'A','B','C' et 'D'. La valeur spéciale `POISON_PILL` indique qu'il s'
agit de la dernière valeur que le processeur recevra (et donc le processeur doit terminer). Le processeur reçoit les
tâches de la blocking queue qui lui est affectée. Il suffit de lire les tâches au fur et à mesure qu'elles arrivent et
de les exécuter (les fonctions des tâches), ainsi que leur coût théorique, sont déjà définies. Une autre fonctionnalité
importante de cette fonction est qu'elle doit jouer le rôle de comptable du temps dépensé par le processeur. Le temps
d'exécution théorique des tâches (disponible en définitions dans `main.c`) doit être utilisé, en plus du temps réel (et
du temps d'attente!) pour mettre à jour les variables de temps dans le processeur.


### Exercice 2.4

Cet exercice est l'un des plus importants de ce TP. Vous devrez écrire le code dans le bloc lexical
qui suit le délimiteur suivant:
```
/// ------------------------------------------------------------------
///           EXERCICE 2.4 DANS LE BLOC LEXICAL SUIVANT
/// ------------------------------------------------------------------
{
    // ICI!
}
/// ------------------------------------------------------------------
///                NE PAS TOUCHER APRÈS CETTE LIGNE
/// ------------------------------------------------------------------
```
Le rôle de ce bloc est de lire les tâches reçues (vous êtes dans une thread différentes de main) au
travers la blocking queue `q`. Ces tâches doivent être affectées aux processeurs pour qu'elles soient
exécutées. Votre travail est d'implémenter l'algorithme qui choisit à quel processeur affecté quelle
tâche, et d'affecter cette tâche au processeur en le plaçant dans la queue de celui-ci. La dernière
tâche, une valeur `POISON_PILL` est placée automatiquement, donc vous n'avez pas à vous préoccuper
de terminer les processeurs.

Vous serez évalué sur la performance de votre algorithme. 50% des points de cette questions sont tirés du bon fonctionnement de celle-ci. La deuxième portion des points est issue du partage des tâches. Une note de 0
(sur cette partie) sera donnée à une solution qui affecte toutes les tâches à un unique processeur. Un note de 50% sera donnée à une solution 
naïve qui partage à tous les processeur. Une note de 100% sera donnée à un algorithme qui maximise l'utilisation des processeurs.

## Remise

Ce travail est à faire **en équipe**. Le code est à remettre sur Github Classroom (autrement dit,
la dernière version à la date de la remise sera utilisée).

Chaque jour de retard est -15%, mais après le deuxième jour la remise ne
sera pas acceptée.

Indiquez clairement votre/vos noms dans le fichier `names.txt` tel qu'indiqué dans le repo du [TPX](https://github.com/IFT2245/TPX).

Le programme doit être exécutable sur les ordinateurs du DIRO. Assurez-vous que tout fonctionne correctement sur les ordinateurs du
DIRO.

## Barême

-   Votre note sera divisé équitablement entre chaque question, sauf la
    question 2.4 qui vaut l’équivalent de 2 questions. 

-   Tout usage de matériel (code ou texte) emprunté à quelqu’un d’autre
    (trouvé sur le web, etc.) doit être dûment mentionné, sans quoi cela
    sera considéré comme du plagiat. Si pour une question votre solution
    est directement copiée, même si il y a attribution de la source,
    cette question se verra attribuée la note de zéro. Vous pourrez
    cependant l’utiliser dans les sections suivantes sans pénalité.

-   Votre devoir sera corrigé automatiquement en très grande partie. Si
    vous déviez de ce qui est demandé en output, les points que vous
    perdrez seront perdus pour de bon. Si vous n’êtes pas certains d’un
    caractère demandé, demandez, et nous répondrons
    de façon à ce que chaque étudiant puisse voir la réponse.

-   La méthode de développement recommandée est d’utiliser CLion et son
    intégration avec Valgrind. Si vous voulez utiliser d’autres
    techniques, vous pouvez le faire, mais nous ne vous aiderons si vous
    rencontrez des problèmes avec ces techniques.

- Les barèmes standards du [TPX](https://github.com/IFT2245/TPX) s'appliquent (fuites mémoires, accès illégaux, etc).

