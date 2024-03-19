# Auteurs : Aurélien CORBLIN & Kévin BENOIT

## RAPPORT : PROJET ANALYSE SYNTAXIQUE

### INTRODUCTION/ CONSIGNE :

- Créer un Lexer qui reconnait les différents éléments du langage tpc.
- Étendre la grammaire de tpc qui devra pouvoir reconnaître les tableaux.
- Pouvoir afficher l'arbre abstrait du code source d'un programme tpc.
- Implémanter les options -t, --tree : pour afficher les arbres.
- Implémanter les options -h, --help : pour afficher l'aide.
- Faire un jeux de tests complet, le dossier test, qui contient le dossier "good" et "syn-err".
- Résoudre un conflit.

### LES PROBLÈMES RENCONTRÉS

Le lexer RAS. 
Le parser et les tableau RAS.
Les arbres : au début, on ne savait pas quand utiliser siblings. 

### COMMENT ON LES A SURMONTÉ 

Pour résoudre le conflit on a utilisé except(1), c’est la meilleure solution que bison propose par défaut.
Pour les arbres on a du modifié la structure de donnée dans tree.h et tree.c pour que cela corresponde a ce que l'on veuille faire.

### LES OPTIONS 

on a fais avec la redirection uniquement.
Si il y a un -- help ou -h on affiche les aides et on ignore tout ce qu’il y a dans la ligne de commande.

### LES TESTS

après avoir fais make, lancer dans le terminal: sh test.sh
