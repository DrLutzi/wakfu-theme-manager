# Wakfu Theme Manager (WTM)

WTM is a desktop application used to manage themes for the game [Wakfu](https://www.wakfu.com). It cannot be used to edit themes, but rather to efficiently download, update, import, manage and combine fan-made themes.

README in other languages: [Français](README_fr.md)

## Changelog -- 1.1

* Fixed bitly links.

## Installation

Download the appropriate binaries for your machine bellow, unzip the archive, and open wtm.exe.

[**Version 1.1 -- Windows 7, 8, 10, 11 - 64-bit**](https://seafile.unistra.fr/f/bbf689dd6b5244f182a6/?dl=1)

## Requirements

The application requires up to about 600 MB of RAM when compiling themes and takes as much disk space as you have downloaded themes, plus the size of Ankama's default theme. Additionally, for zip/unzip/download features, on Windows, *powershell 3.0 or more* must be installed, and on other OS, *unzip* must be installed.

## Usage

### First use

On opening the application, Ankama's default theme and json file will be downloaded and imported in ./themes.

If the application cannot find your game's "theme" folder, you will be asked to provide it. 
If you do not provide it, you will have to open the settings pannel to change it.

If the application downloaded the default theme correctly, a new disabled theme called "default" should appear in the main window, and it should take the default dungeon image as icon.

### Importing themes

**WARNING: DO NOT COPY FILES DIRECTLY INTO THE WORKING FOLDER "themes" OF THE APPLICATION. Also, please use backups when using this application.**
There are three ways to import a theme, corresponding to the three first action buttons.

**Import folder**: imports a folder containing a theme, or a folder containing several theme folders (in which case there will be as many themes imported as there are valid theme folders).

![importDirectory](https://user-images.githubusercontent.com/15910330/149378815-b48b6736-5ab6-4a08-822d-c9425f2ef56d.gif)

**Import zip**: imports a .zip archive containing a theme. If this does not work, make sure you meet the requirements (and please notify me on discord: Lutzi#3767).

![importZip](https://user-images.githubusercontent.com/15910330/149379104-259bc53a-7e5c-4bdd-9be8-59294d25736f.gif)

**Import link**: imports a link where your theme is stored, as a .zip archive. Again, if this does not work for some links, please notify me.

![importUrl](https://user-images.githubusercontent.com/15910330/149379061-8731d136-7836-4e59-b3b2-84db62e82b97.gif)

Once you imported a theme, it should appear in the leftmost list.

### Removing themes

You cannot remove themes from the application. To remove a theme, close the application, go to the "themes" folder of the application and remove the folder corresponding to the theme you do not want to see anymore.

### Manipulating themes

To manipulate themes, you have to drag and drop them around. There are two lists for themes: the one on the left is a view of the themes you downloaded; the one on the right represents which themes you want to use to  create the theme that will be used by your game.

Each theme is represented by its icon, its name (the name of the folder), and a link pointing to the theme for updates. You can provide a link even if you provided it from your disk. To update the theme, press the download button on the theme view.

### Compiling themes

Themes placed on the rightmost list will be used when compiling a final theme. **In most case, you want only one theme to be present on the rightmost list**.
The following actions are associated with this category:

**Make theme**: compiles the theme with all themes on the rightmost list and exports it to the output specified in the parameters (defaults to your own game's theme folder).

![makeTheme](https://user-images.githubusercontent.com/15910330/149379227-95d5b4bd-1962-425c-ba8d-babfc793f153.gif)

**Clear theme**: deletes theme files in the output specified in the parameters (defaults to your own game's theme folder).

![clearTheme](https://user-images.githubusercontent.com/15910330/149379263-4ecefbae-408f-47de-9230-2742b8f97695.gif)

### Fusing themes

If several themes are placed in the rightmost list when compiling themes, they will be merged in the final theme. Themes are attributed an importance depending on their order on the list, from top to bottom, top being the most important. For instance, if you have two themes A and B and A is on top of B:
* If A has a custom color or pixmap that B does not have, A will override it;
* If B has a custom color or pixmap that A does not have, B will override it;
* If A and B have a custom color or pixmap, A will override it.
* All pixmaps identical to those of the default theme of Ankama in A or B are detected and not taken in account.

This feature is designed to be used with a macro theme along with one or several micro themes. For instance, the following example takes the dark flat theme of Augaroma (badly spelled) and a theme that modifies only the HP heart's background located in *themeSimple.png*.

![fuseThemes](https://user-images.githubusercontent.com/15910330/150188934-ac94a1a0-bde2-459c-99fc-cbf660a415a6.gif)

*Note*: if this feature does not seem to work correctly, make sure your micro themes do not modify default neighbouring pixmaps, especially through an image editing application. WTM authorizes a very small accidental difference on each pixel, but any modification above this threshold will make the corresponding pixmap count as a modified pixmap.

### Other actions

The following actions are associated with this category:

**Update Ankama**: updates or downloads Ankama files, including the json file and the default theme files. This action is rarely used.

**Settings**: opens a panel with the settings. **Most of the time, you do not need to modify settings**. If the program detects that a setting seems wrong, it will be highlighted in yellow.

## Contributing

**Themes**

All themes are compatible with the application. Here are, however, some recommendations for an optimal usage:
* Make sure to provide a downloadable link to your theme as a .zip archive (no .rar, no folders to navigate like in dropbox). *Note*: github repositories are valid (as they provide a download link in .zip).
* You can add a squared image named *theme.png* at the root of the theme. This image will represent the icon of your theme. *Note*: if no image is provided, the texture *dungeon.png* will be used instead; if it is not present, a square by default will be used (like in the demos).

**Code**

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change. If you have any suggestion but are not familiar with coding C++, please open an issue.

### Compilation

You will need [Qt](https://www.qt.io/) 5.14 or more. Use of QtCreator is highly recommended since it is bundled with a C++ compiler on Windows, QDesigner, used to design the GUI, and QLinguist, used to translate the application in French.

To compile:

```console
qmake wtm.pro
make
cp config/config.json <build directory>
```

Or setup the project in QtCreator and copy the configuration file in the build directory.

## License
[GNU AGPL](https://choosealicense.com/licenses/agpl-3.0/)
