# Wakfu Theme Manager (WTM)

WTM est une application de bureau utilisée pour gérer les thèmes d'interface personnalisés du jeu [Wakfu](https://www.wakfu.com). Elle ne peut pas être utilisée pour éditer les thèmes, mais plutôt pour télécharger, mettre à jour, importer, gérer et combiner des thèmes personnalisés.

## Installation

Téléchargez les fichiers pour votre jeu sur le README principal (en anglais).

## Prérequis

L'application a besoin de jusqu'à 600 Mo de RAM en construisant les thèmes, et prend autant de mémoire sur le disque que vous avez téléchargé de thèmes, plus la taille du thème par défaut d'Ankama.
Additionnellement, pour les téléchargements et l'ouverture d'archives, sur Windows, *powershell 3.0 ou plus* doit être installé, et sur les machines Unix, *unzip* doit être installé.


## Utilisation

### Premiers pas

En ouvrant l'application, le thème par défaut d'Ankama et le fichier json des thèmes vont être téléchargés dans le dossier "themes" et importés dans l'application.

Si l'application ne peut pas trouver le dossier "theme" du jeu automatiquement, elle va vous demander de l'ouvrir. Si vous ne l'ouvrez pas, vous devrez ouvrir les options pour le changer.

Si l'application a téléchargé le thème par défaut correctement, un thème grisé appelé "default" devrait apparaitre dans la fenêtre principale, et devrait prendre l'image de donjon du jeu par défaut comme icône.

### Importer des thèmes

Il y a trois façons d'importer un thème, correspondant aux trois premiers boutons d'actions de la barre d'actions. Notez que les thèmes sont copiés dans le dossier "themes" de l'application à l'import (pas du jeu).

**Importer dossier** : importe un dossier contenant un thème, ou un dossier contenant plusieurs dossiers de thème (auquel cas autant de thèmes sont importés que de dossiers de thèmes valides). 

![importDirectory](https://user-images.githubusercontent.com/15910330/149378815-b48b6736-5ab6-4a08-822d-c9425f2ef56d.gif)

**Importer archive** : importe une archive .zip contenant un thème. Si cette action ne fonctionne pas, assurez-vous que votre machine a les prérequis (et/ou notifiez l'auteur sur discord : Lutzi#3767).

![importZip](https://user-images.githubusercontent.com/15910330/149379104-259bc53a-7e5c-4bdd-9be8-59294d25736f.gif)

**Importer lien** : importe un lien vers un thème stocké sur un serveur *en tant qu'archive .zip*. Si ça ne fonctionne pas pour certains liens, prévenez l'auteur également.

![importUrl](https://user-images.githubusercontent.com/15910330/149379061-8731d136-7836-4e59-b3b2-84db62e82b97.gif)

Une fois un thème importé, il devrait apparaître dans la liste de gauche.

### Manipuler les thèmes

Pour manipuler les thèmes, vous devez les *glisser-déposer*. Il y a deux listes pour les thèmes : celle de gauche est une vue des thèmes téléchargés et importés; celle de droite représente les thèmes que vous voulez utiliser pour construire le thème final pour votre jeu.

Chaque thème est représenté par son icône, son nom (qui est aussi le nom du dossier), et un lien internet facultatif pour les mises à jour du thème. Vous pouvez fournir un lien même si le thème vient à l'origine de votre disque. Pour mettre un thème à jour, appuyez sur le bouton de téléchargement de la vue du thème.

### Construire un thème

Les thèmes placés sur la liste de droite vont être utilisés pour construire un thème final. **Dans la plupart des cas, vous ne voulez qu'un seul thème sur la liste de droite**. Si plusieurs thèmes sont placés dans cette liste, leur importance est classée du haut vers le bas, le thème le plus haut étant le plus important. Par exemple, si vous avez deux thèmes A et B et que *A est au-dessus de B* :
* Si A a une couleur ou une pixmap personnalisée que B n'a pas, celle de A va être utilisée pour le thème final;
* Si B a une couleur ou une pixmap personnalisée que A n'a pas, celle de B va être utilisée pour le thème final;
* Si A et B ont tous deux une couleur ou une pixmap personnalisée, *celle de A va être utilisée pour le thème final*.

Les actions suivantes sont associées à cette catégorie :

**Construire thème** : construit le thème final avec tous les thèmes de la liste de droite, et l'exporte vers le dossier spécifié dans les paramètres (par défaut, le dossier est détecté et est le dossier de thème de votre jeu Wakfu).

![makeTheme](https://user-images.githubusercontent.com/15910330/149379227-95d5b4bd-1962-425c-ba8d-babfc793f153.gif)

**Nettoyer thème** : supprime tous les fichiers de thème du dossier spécifié dans les paramètres (par défaut, le dossier est détecté et est le dossier de thème de votre jeu Wakfu).

![clearTheme](https://user-images.githubusercontent.com/15910330/149379263-4ecefbae-408f-47de-9230-2742b8f97695.gif)

### Autres actions

Les actions suivantes sont associées à cette catégorie :

**MAJ Ankama** : met à jour ou télécharge les fichiers d'Ankama (le fichier json et les fichiers du thème par défaut). Cette action est rarement utilisée.

**Options**: ouvre un panneau avec les options. **Dans la majorité des cas, vous n'avez pas besoin de modifier ces options**. Si le programme détecte qu'une option semble être fausse (par exemple un dossier qui n'existe pas), l'option aura une apparence jaune à la place de vert.

## Contribuer
**Thèmes**

Tous les thèmes sont compatibles avec l'application. Voici cependant quelques recommendations pour une utilisation optimale :
* Faites en sorte de fournir un lien vers le thème téléchargable sous le format .zip (pas de .rar, pas de dossiers à parcourir comme sur dropbox). *Remarque* : les dépôts github sont valides (puisqu'il est possible de télécharger le thème en .zip).
* Facultatif : rajoutez une image carrée nommée theme.png à la racine du thème. Cette image représentera l'icône du thème. *Remarque* : si aucune image n'est fournie, la texture *dungeon.png* sera utilisé à la place si elle est présente, sinon, un carré par défaut (comme dans les démonstrations).

**Code**

Les *pull requests* sont bienvenues. Pour des changements majeurs, ouvrez svp une *issue* en premier lieu pour discuter ce que vous voudriez changer. Si vous avez une suggestion mais n'êtes pas familié avec le C++, ouvrez une *issue* svp.

### Compilation

Vous avez besoin de [Qt](https://www.qt.io/) 5.14 ou plus. L'utilisation de QtCreator est hautement recommandée puisque ce programme inclut un compilateur C++ sur Windows, QDesigner qui est utilisé pour construire l'interface graphique, et QLinguist, qui est utilisé pour traduire l'application en français et en anglais.

Pour compiler :

```console
qmake wtm.pro
make
cp config/config.json <build directory>
```

Ou configurez simplement le projet dans QtCreator, et copiez le fichier de configuration *config/config.json* dans le répertoire de compilation.

## Licence
[GNU AGPL](https://choosealicense.com/licenses/agpl-3.0/)

