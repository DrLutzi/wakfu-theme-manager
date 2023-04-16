# Wakfu Theme Manager (WTM)

WTM is a desktop application used to manage themes for the game [Wakfu](https://www.wakfu.com). It cannot be used to edit themes, but rather to efficiently download, update, import, manage and combine fan-made themes.

README in other languages: [Français](README_fr.md)

## Changelog -- 2.0

* The application now allows users to download and install community themes with a single button.
* The merging of several themes was removed to decrease the complexity.

## Installation

Download the appropriate binaries for your machine bellow, unzip the archive, and open wtm.exe.

[**Version 2.0 -- Windows 7, 8, 10, 11 - 64-bit**](https://www.dropbox.com/s/hjz7ipk6kw6rikj/WTM_2_0.zip?dl=1)

## Requirements

On Windows, *powershell 3.0 or more* must be installed, and on other OS, *unzip* must be installed.

## Usage

### First use

If the application cannot find your game's "theme" folder, you will be asked to provide it.
Otherwise, the game's "theme" folder will be written on the status bar.

### Working with themes

**WARNING: DO NOT COPY FILES DIRECTLY INTO THE WORKING FOLDER "themes" OF THE APPLICATION, AND DO NOT CREATE YOUR CUSTOM THEMES DIRECTLY IN YOUR GAME'S THEME FOLDERS. Please use backups when using this application: while work was done to limit the risk of lost data, the application HAS to occasionally remove some files and folders in order to work properly.**

![wtm_tuto](https://user-images.githubusercontent.com/15910330/232262445-7e62a5a3-8c0a-4ac2-83ee-590c783549a7.png)

1. **Export folder**: copies and paste a folder containing a theme into the game's "theme" folder.
2. **Export Zip**: imports a .zip archive containing a theme. If this does not work, make sure your archive meets the requirements described in #community-themes.
3. **Reset theme**: removes the "colors" and "images" folder in your game's theme folder. You will be prompted for a confirmation.
4. **Dowmload default theme**: for creators, downloads the latest theme files of Ankama into themes/ankama.
5. **Settings**: access two settings: the path to the game (useful if you have several installs), and the link to Ankama's json (if it ever changes).
6. *Forum link*: opens the link to the forum in your web browser.
7. *Download*: downloads the theme and puts it in your game's "theme" folder.

### Community themes

The main list shows you themes that I registered. The list is automatically updated when I update it remotely. If you want more themes shown, contact me on discord (Lutzi#3767) with a link to the forum page of the custom theme you want.

**Creators**: 

For your theme to be on the list, please make sure of the following:
* You made a forum post with your download link.
* The theme has the form of a .zip file that can be downloaded.
* The theme archive does not contain another archive.

If you submit a theme to me, you can attach a 128x128 picture representing your theme.

### Other actions

**Code**

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change. If you have any suggestion but are not familiar with coding C++, please open an issue.

### Compilation

You will need [Qt](https://www.qt.io/) 5.14 or more. Use of QtCreator is highly recommended since it is bundled with a C++ compiler on Windows, QDesigner, used to design the GUI, and QLinguist, used to translate the application in French.

To compile (Linux, Windows, macOS):

```console
qmake wtm.pro
make
cp config/config.json <build directory>
```

Or setup the project in QtCreator and copy the configuration file in the build directory.

## License
[GNU AGPL](https://choosealicense.com/licenses/agpl-3.0/)
