#+title: Taxicab
[[file:20201205215419-project.org][#project]]
[[file:#universita.org][#universita]]

- [[file:/home/dan/Documents/UNI/II/SO/PROGETTO.pdf][Documento di Presentazione del Progetto]]

  + Opzioni di compilazione
    - gcc -std=c89 -pedantic

  + utilizzo di make

- [[file:home/dan/Documents/UNI/II/SO/taxicab-overview.pdf][Overview]]

* Struttura della Simulazione


** Master
[[file:/home/dan/Code/C/Taxicab/master.c][File]]
- raccoglie stats
- ogni secondo stampa lo stato di occupazione delle celle
- alla fine stampa
  + n viaggi
    - success
    - unserved
    - aborted
  + mappa
    - sorgenti
    - holes
    - top cells
      + piú attraversate
  + processo taxi
    - maggior strada percorsa
    - viaggio piú lungo
    - servito il maggior numero di richieste


*** Generator
[[file:/home/dan/Code/C/Taxicab/generator.c][File]]
1. Gestisce Configurazione
2. Genera Mappa
   - una matrice di struct =cell=
   - (x,y)
   - type - char
     + SOURCE
     + HOLE
     + FREE
   - capacity - int
     + SO_CAP_MIN <= capacity <= SO_CAP_MAX
   - traffic
     + <= capacity
   - n_visite - int
     + incrementato da ogni taxi di passaggio
3. Genera SO_TAXI processi figli
   - execv taxi.c
     + indica uno spawn casuale
       - non HOLE
       - una cella che non abbia ecceduto la sua CAPACITY
4. Predispone Msg Queue
   - =msg=
     + destinazione (x,y)
   - dimensione
5. Genera Richieste
6. Inserisce Richieste
7. Una volta passato SO_DURATION
   - termina tutti i processi
   - SIGINT a master
     + che stampa le statistiche prima di terminare
*** Taxi(s)
[[file:/home/dan/Code/C/Taxicab/taxi.c][File]]
1. Si sposta nella Source libera piú vicina
   * una volta scelta aumenta subito il suo Traffic
     + in modo che i taxi non si rubino il posto a vicenda
   * se non ne trova si sposta di una cella verso la piú vicina source e riprova
2. viariabile timeout

** Config
[[file:/home/dan/Code/C/Taxicab/taxicab.conf][File]]
- SO_TAXI
  + n taxi
- SO_SOURCE
  + n sources
- SO_HOLES
  + n holes
- SO_CAP_MIN
- SO_CAP_MAX
  + Capacitá MIN e MAX di ogni cella
    - ogni cella ha capacitá casuale
- SO_TIMENSEC_MIN
- SO_TIMENSEC_MAX
  + MIN e MAX tempo di attraversamento di ogni cella
- SO_TIMEOUT
  + ogni processo taxi si chiude dopo questo tempo di inattivitá
- SO_DURATION
  + dopo questo tempo il processo Generator invia SIGINT ai figli
- SO_WIDTH
  + X
- SO_HEIGHT
  + Y
