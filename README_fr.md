# Wakfu Theme Manager (WTM)

WTM est une application de bureau utilisée pour gérer les thèmes du jeu [Wakfu](https://www.wakfu.com). Elle est utilisée pour facilement télécharger et mettre à jour les thèmes faits par la communauté.

## Installation

Téléchargez les fichiers et l'exécutable pour votre machine, décompressez l'archive et ouvrez l'exécutable.

[**Release 2.1.1 -- Windows 7, 8, 10, 11 - 64-bit**](https://www.dropbox.com/scl/fi/tfbuwgus5kfy9i2s46pfd/WTM_2_1_1.zip?rlkey=r2tvdhijm5b34gynovbz0425y&dl=1)

## Requirements

Pour Windows, *powershell 3.0 ou +* doit être installé (c'est le cas par défaut), et dans d'autres systèmes d'exploitation, *unzip* doit être installé.

## Changelog

**2.1.1**
* Correction d'une erreur de désarchivage pour Windows 11.

**2.1.0**
* Changement du chemin vers le jeu, en accordance avec le nouveau.

**2.0.1**
* Correction du téléchargement pour certaines versions de Windows.

**2.0.0**
* Le design de l'application a été modifiée et permet maintenant de télécharger les thèmes avec un seul bouton.
* La fusion de thèmes a été retirée pour réduire la complexité.

## Utilisation

### Première utilisation

Si l'application ne parvient pas à trouver le dossier theme de votre jeu (ce qui est inhabituel), vous serez invité à le fournir. Sinon, le dossier theme du jeu sera affiché dans la barre d'état.

### Utiliser les thèmes

<img src="https://user-images.githubusercontent.com/15910330/232262445-7e62a5a3-8c0a-4ac2-83ee-590c783549a7.png" width="500">

1. **Exporter un dossier**: copie et colle un dossier contenant un thème dans le dossier theme du jeu.
2. **Exporter une archive Zip**: importe une archive .zip contenant un thème. Si cela ne fonctionne pas, assurez-vous que votre archive répond aux exigences décrites dans #community-themes.
3. **Réinitialiser le thème**: supprime les dossiers `colors` et `images` dans le dossier de thème de votre jeu. Vous devrez confirmer cette action.
4. **Télécharger le thème par défaut**: pour les créateurs, télécharge les derniers fichiers de thème d'Ankama dans le dossier `themes/ankama`.
5. **Paramètres**: accédez à deux paramètres : le chemin d'accès au jeu (utile si vous avez plusieurs installations) et le lien json d'Ankama (si jamais il change).
6. *Lien vers le forum*: ouvre le lien vers le forum dans votre navigateur Web.
7. *Télécharger*: télécharge le thème et le place dans le dossier theme de votre jeu.

**AVERTISSEMENT : Suppression de fichiers**

L'application devra parfois supprimer les dossiers suivants et leur contenu, de manière récursive :
* Le dossier `images` du dossier `theme` de votre jeu,
* Le dossier `colors` du dossier `theme` de votre jeu,
* Les dossiers de l'archive téléchargée qui sont décompressés dans le dossier `themes` de l'application lors du téléchargement d'un thème.
    Pour éviter toute perte de données, veuillez éviter de placer des fichiers sensibles (en particulier vos propres fichiers de thème) dans ces dossiers.

### Thèmes de la communauté

La liste principale affiche les thèmes que j'ai enregistrés. La liste est automatiquement mise à jour lorsque je la mets à jour à distance. Si vous souhaitez voir plus de thèmes, contactez-moi sur discord (Lutzi#3767) avec un lien vers la page du forum du thème personnalisé que vous souhaitez.

**Créateurs:**
Pour que votre thème figure sur la liste, veuillez vous assurer des éléments suivants :

* Vous avez publié un message sur le forum avec votre lien de téléchargement.
* Le thème prend la forme d'un fichier .zip pouvant être téléchargé.
* L'archive de thème ne contient pas une autre archive.
* Je suis au courant de l'existence de votre thème.

Si vous me soumettez un thème, vous pouvez y attacher une image de 128x128 représentant votre thème.

## Code

Les *pull requests* sont les bienvenues. Pour les changements majeurs, veuillez ouvrir une *issue* d'abord pour discuter de ce que vous souhaitez modifier. Si vous avez une suggestion mais n'êtes pas familiarisé avec la programmation en C++, veuillez ouvrir une *issue*.

### Compilation

Vous aurez besoin de [Qt](https://www.qt.io/) 5.14 ou supérieur. L'utilisation de QtCreator est fortement recommandée car il est fourni avec un compilateur C++ sur Windows, QDesigner, utilisé pour concevoir l'interface graphique, et QLinguist, utilisé pour traduire l'application en français.

Pour compiler sur Linux et macOS sans QtCreator :

```console
qmake wtm.pro
make
```

Avec QtCreator, compilez le projet, puis copiez le fichier de configuration dans le dossier de compilation.

### Traduction
Si vous avez besoin des fichiers de traduction français, vous pouvez les compiler avec `lrelease` et copier le fichier `.qm` dans le dossier de votre application.

## Licence
[GNU AGPL](https://choosealicense.com/licenses/agpl-3.0/)
