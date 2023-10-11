# Wakfu Theme Manager (WTM)

WTM is a desktop application used to manage themes for the game [Wakfu](https://www.wakfu.com). It is used to efficiently download and update community-made themes.

**README in other languages: [Français](README_fr.md)**

## Installation

Download the appropriate binaries for your machine bellow, unzip the archive, and open wtm.exe.

[**Release 2.1.0 -- Windows 7, 8, 10, 11 - 64-bit**](https://www.dropbox.com/scl/fi/t2ccpp5w1iu3qhq3lwz3s/WTM_2_1_0.zip?rlkey=kaf765wyrpbptblnz127soxyf&dl=1)

## Requirements

On Windows, *powershell 3.0 or more* must be installed (which is the case by default), and on other OS, *unzip* must be installed.

## Changelog

**2.0.1**
* Fixed the download for some versions of Windows.

**2.0.0**
* The application now allows users to download and install community themes with a single button.
* The merging of several themes was removed to decrease the complexity.

## Usage

### First use

If the application cannot find your game's `theme` folder (unusual), you will be asked to provide it.
Otherwise, the game's `theme` folder will be written on the status bar.

### Working with themes

<img src="https://user-images.githubusercontent.com/15910330/232262445-7e62a5a3-8c0a-4ac2-83ee-590c783549a7.png" width="500">

1. **Export folder**: copies and paste a folder containing a theme into the game's `theme` folder.
2. **Export Zip**: imports a .zip archive containing a theme. If this does not work, make sure your archive meets the requirements described in #community-themes.
3. **Reset theme**: removes the "colors" and "images" folder in your game's theme folder. You will be prompted for a confirmation.
4. **Dowmload default theme**: for creators, downloads the latest theme files of Ankama into themes/ankama.
5. **Settings**: access two settings: the path to the game (useful if you have several installs), and the link to Ankama's json (if it ever changes).
6. *Forum link*: opens the link to the forum in your web browser.
7. *Download*: downloads the theme and puts it in your game's `theme` folder.

**WARNING: Removal of files**

The application will occasionally need to remove the following folders and all of its contents, recursively:
* The `images` directory in your game's `theme` directory,
* The `colors` directory in your game's `theme` directory,
* The downloaded archive's folders that it decompresses into the `themes` folder of the application when downloading a theme.
To ensure that you do not lose data, please avoid putting anything sensitive (especially your own theme files) into these folders.

### Community themes

The main list shows you themes that I registered. The list is automatically updated when I update it remotely. If you want more themes shown, contact me on discord (Lutzi#3767) with a link to the forum page of the custom theme you want.

**Creators:**

For your theme to be on the list, please make sure of the following:
* You made a forum post with your download link.
* The theme has the form of a .zip file that can be downloaded.
* The theme archive does not contain another archive.
* I am aware that your theme exists.

If you submit a theme to me, you can attach a 128x128 picture representing your theme.

## Code

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change. If you have any suggestion but are not familiar with coding C++, please open an issue.

### Compilation

You will need [Qt](https://www.qt.io/) 5.14 or more. Use of QtCreator is highly recommended since it is bundled with a C++ compiler on Windows, QDesigner, used to design the GUI, and QLinguist, used to translate the application in French.

To compile on Linux and macOS without QtCreator:

```console
qmake wtm.pro
make
```

With QtCreator, compile and copy the configuration file in the build directory.

### Translation
Should you require the French translation file, you can compile it with lrelease and copy the created .qm file into your application's folder.

## License
[GNU AGPL](https://choosealicense.com/licenses/agpl-3.0/)
