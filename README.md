#BAGPlug-SCDS2

A historical archive for a project I created prior to any formal programming education.

![Alt Text](/screenshots/11-3-24-17-4-7.jpg?raw=true "BAGPlug Cover Art")
![Alt Text](/screenshots/11-3-24-17-11-11.jpg?raw=true "BAGPlug Cover Art 2")
![Alt Text](/screenshots/11-3-13-17-48-42.png?raw=true "BAGPlug Media Player")
![Alt Text](/screenshots/11-3-14-6-18-12.png?raw=true "BAGPlug Open Context Menu")
![Alt Text](/screenshots/11-3-13-16-29-59.png?raw=true "BAGPlug Skin Selector")




##Original Readme:

BAGPlug - BassAceGold

Third Release (change log below)

A replacement plugin menu for the Supercard DSTwo


###Installation
- (OPTIONAL RECOMMENDED) format sd card to FAT16/FAT (cards 4gb or less)
- Place _bagui folder on root of card
- Copy DSTwo.nds and boot.ini to _dstwo folder on root of card
- (OPTIONAL) Configure emulator support via arg files (See "Adding new file support:" below). Binary locations may need
to be updated to their current path on your card.



###Skinning support
- png files, 8 bit, 16 bit, 24 bit and 32 bit bitmap files. *NOTE*: backgrounds must be 16 or 24 bit bmp so fast rendering can be used for a smoother browser
- Windows and sub windows are just color codes defined via the skin.ini file. These color values are 16 bit colors, the included ColorConvert.exe program can be used for converting 24 bit RGB color values to 16 bit color values (PA_RGB in the program)


###Custom icons
- uses 32 x 32 bitmap or png files
- must be named the same as the file with and in the same directory OR they can be placed in a directory as defined in the current skin.ini under "Custom_Icon_Folder".

eg:
BAGSFC.plg - the file you want to have a custom icon
BAGSFC.plg.png/BAGSFC.plg.bmp- the icon file


###Game Covers
- 16 bit and 24 bit bitmaps, or any format of png file supported. Transparency effects will not be used for performance purposes
- can be any size as long as it fits on screen
- can be either png or bmp files
- must be named exactly the same as the game file including the file extension (see note below for commercial roms)
- CoverArt.ini file can be present in the same directory as the media/game files (it has a higher priority over global settings) or a global CoverArt.ini file can be used in /_bagui/user files/

###ini file example:
```
[file extension] -file extension must be replaced with the file type you wish to add covers for
#path to folder with covers
folder = /_bagui/cover art/nds
#image type
type = png
```

*Commercial NDS roms use the game ID for covers and do not need the file extension in the title of the cover file.



###Media Player
- Mp3, Ogg, Wav supported.
- All songs in current directory as the current song being played are automatically queued for automatic track changes
- Music player can be backgrounded (Press X or the backgrounding icon) for file browsing while listening to tunes
- an audio file can be set to launch on boot. Press select on a supported audio file and select "set boot song" option.
- a boot song can be removed by selecting the option to do so when select is pressed on any supported audio file.


###Settings menu
- Allows to instantly swap skins with no rebooting
- Contains settings for various display options
- contains settings for setting CPU clock speeds for various actions

###Hiding Files / Folders
- Files and Folders can be hidden from view simply by adding them to the hiddenfiles.txt and hiddenfolders.txt files in /_bagui/user files/ folder
- Files and Folders can also be hidden while in the menu by pressing select on the desired file or folder, and selecting the hide option.


###Supported File formats
- Standard supported files include PLG, NDS (homebrew and commercial via EOS 1.11), SNES file formats (including zip archives), Mp3, Wav, Ogg, .mpc (mario paint composer DS files), .bmp (Etch)
- New file formats can be added via arg files (in /_bagui/ext folder) for programs which can accept boot args
- File formats can be set to open in multiple different programs. eg: a zip file can contain a snes rom or a gba rom, a pop up will come up and ask which program to launch said file in. This program list can be edited in the arg files for the specified file type


###Adding new file support
Programs that support launch arguements can be used open specific files selected within BAGplug. For example an emulator such as nesDS which can open .nes files.

*Example setup using nesDS*
First, a .arg file must be created in /_bagui/ext/ which will contain the instructions for launching .nes files within the menu. 
The name format of .arg files is quite simply the extension of the file format to support with a .arg extension. In this case, the file will be called "nes.arg".

Format of .arg files
```
nesDS://Title of the program
"/_bagui/nesDS/nesDS.nds"//file path to program
[$PRGMPATH$,$FILEPATH$]//launch arguments
//-EOSSETTINGS//use default supercard menu to launch the file if necessary
;
```

- $PRGRMPATH$ will pass on the file location of the launched program to the program itself, this is standard.
- $FILEPATH$ will pass on the rom location from the menu to the program, this is what is needed for the program to load the correct file.
- There are no limits as to how many arguements to pass on to a program however the program will only take what it needs.
- Order of args may vary depending on programs, so be sure to watch out for that
- The -EOSSETTINGS flag can be added to program arg which will have BAGplug use EOS (1.11 and up) to launch the file. This can be used for homebrew that have issues with the default launcher, or if you would want to multiboot different versions of EOS.

More than one program can be added to a .arg file. When the file is launched, 
the open with dialog will open allowing you to choose which program to use. See smc.arg for an example

Individual arg files can be created for specific files and will have priority over global arg files. 
These files must be in the same directory with the same name as the file eg. super mario kart.smc / super mario kart.smc.arg

###Favorites menu
- Any supported file type can be linked in this menu (snes roms, zip files, nds roms, image files, ogg and wav files, etc)
- Favorites list is just a text file containing file paths to all the files you want to save for quick launching
- Can add files to the favorites list via the normal browser, or remove files within the favorites menu (select button)


###Copy/Cut/Paste/Delete
- Pressing /select/ on a file or folder brings up the file operations menu where you can selected an option
- Files will be pasted in the current directory you are browsing regardless of what is high-lighted

###Input
-All menu have some sort of pad and stylus input
-if L and R buttons are pressed, you can take a screenshot of the both screens

###NZIP
- NDS roms can be zipped with NitroZip to save space on the SD card when the roms are not in use.
- BAGPlug keeps track of which roms are unzipped, these roms can be viewed by clicking the nzip icon on the side bar. This icon only appears when at least 1 rom has been unzipped
- Once a rom is unzipped, it won't need to be unzipped again unless it is deleted in the NZIP manager.
- There is no limit to how many roms can be left unzipped at a time, it is up to the user to ensure there is enough free space on the SD card to unzip future roms.



###Change log

Release 3 - 5:
- top screen is now force updated when taking screen shots
- screen shots are now saved in */_bagui/screen shots/
- screens are now turned off, then on again when taking screen shots (screen shot feedback)
- new screen shot naming scheme
- improved coverart.ini organization

Release 3 - 4:

Includes fixes 3 - 1
- fixed copy/cut & paste corruption

Includes fixes 3 - 2
- fixed boot song not loading
- fixed a stylus input error in nzip manager
- fixed unzipped files not being added to unzipped.txt
- slightly optimized nds icon loading

3-4 changes:
- _bagui folder can now be placed anywhere on the SD card, as long as the DSTwo.nds file from this release is used for loading
- improved failsafe launching in DSTwo.nds
- updated manual.txt

*Notes*
Make sure to update all the required file paths when moving the _bagui folder (boot.ini, *.arg files, skin.ini files, etc)

Release 3 - 3:
- can now hide files and folders within BAGplug
- fixed memory leak in file ops menu
- fixed favorites list refreshing
- contains 3-2 fixes 1 and 2
- all external lists (favorites, hiddenfiles, hiddenfolders, unzipped) are automatically updated based on if the file/folder does not exist or has the wrong path
- error messages for file support are now loaded from language.ini
- Etch and Mario Paint Composer DS are now included and configured

*Upgrade Notes*
- language.ini has been updated


Release 3 - 2:
- fixed exiting of "open with" sub menu
- added -EOSSETTINGS flag for arg files which forces BAGplug to fill in globalsettings.ini for the launched program. (internal launcher will launch the default EOS version installed for commercial roms)

*Upgrade Notes*
- nds.arg has been updated to show use of -EOSSETTINGS flag


Release 3 - 1:
- fixed freezing in favorites when scrolling
- fixed audio skipping when entering directories with lots of files
- fixed memory corruption in DSTwo.nds breaking arg support for .plg files. Should be more robust for various sd cards/formats now
- improved memory management for icons
- fixed stylus input in open with menu
- fixed memory leaks in graphics loading/cloning functions of core library - cleaning up nds icon handling and no file icon corruption
- fixed icon transparecy support
- improved open with handling in favorites menu
- improved performance in favorites menu with game covers
- improved performance in scrolling nds files in browser
- covers for non-commercial nds files require the file extension in the cover name. eg: game.nes / game.nes.png
- improved minimizing from media player to current menu
- fixed freezing when deleting favorites
- fixed crashing when entering EOS Settings
- improved scroll bar handling
- optimized nds icon loading
- optimized directory scanning
- added option to enable nds icon transparency
- improved handling of text lists (favorites, hidden files / folders)
- favorites.txt, hiddenfiles.txt, hiddenfolders.txt, and settings.ini now go in /bagui/user files/ folder
- nzip support added with built in nzip manager for deleting unzipped games 
- nzip cpu speed option added to settings
- optimized top screen refreshing in main browser
- misc bug fixes
- NitroZip tool included
- added "NZIP" section to readme
- modified the "Adding new file support" section to include information on file specific arg files in readme
- updated "Game Covers" section in readme

*upgrade notes*
- New skin parts: delete, exit, nzip, selectall -see BAGplug 3.0 or Default skin for examples
- more text added to language.ini
- favorites.txt, hiddenfiles.txt, hiddenfolders.txt, settings.ini, unzipped.txt all go in /_bagui/user files/
- nzip.arg added to /_bagui/ext/ folder

Release 3:
- fixed eos settings appearing in favorites screen for non-nds files
- unicode strings are now written in extlink.dat
- fixed custom icons not loading when in the same directory as the file
- custom icons now need to include the extension of the file it is for in the file name (BAGSFC.plg / BAGSFC.plg.png)
- improved handling of globalsettings.ini
- added unicode support to text, all text is now loaded externally for easy translations. A new font will need to be created to support all characters
- added stylus double clicking to favorites menu
- fixed browser scrolling glitches after changing skins
- minor UI tweaks
- DSGAME.nds is now recognized as homebrew rather than a commercial game
- new skin: BAGPlug 3.0
- updated DSTwo.nds with homebrew menu 2.0 loader


Release 2 - 9:
- fixed file sorting on root skipping an item
- can toggle whether to hide favorited items in the browser or not. Directory display does not get updated until re-entry. if a new file is added to your favorites (This is not a bug, it prevents lag in folders with lots of files).
- custom icons can either be png or bitmap files and do not need a .ico extension anymore
- improved stylus controls in the settings and open with menus
- added EOS Settings for roms, (press select on a commercial rom or Y in favorites). This lets you switch between clean/patch mode, turn cheats on/off and select one of the multi saves.

Release 2 - 8:
- AM/PM is no longer displayed on 24 hour clock
- no longer tries to open last folder if that folder doesn't exist
- added ability to preview skins - press Y when highlighting a skin the the settings menu ->new section in skin.ini
- enterLastDirWhenBoot in the globalsettings.ini is now set to 1 when launching a rom, auto run in the DSTwo menu requires it set to 1.
- fixed some icon mix ups in the browser

Release 2 - 7:
- added extlink support
- improved internal DS name displaying
- removed the show all files option in settings
- removed hide folders setting in favor of using hiddenfolders.txt
- fixed a memory leak in the favorites menu
- tweaked favorites menu loading
- added hiddenfiles.txt support
- fixed an error in extensions handling in the browser

Release 2 - 6:
- added ability to create a list of hidden folders using "hiddenfolders.txt" in the /_bagui/ folder. One folder is listed per line with no ending '/'. eg. to hide the "_bagui" folder -> /_bagui
- fixed issue with back paging through a directory and icons not loading properly
- fixed issue where DSGAME would display a blank name when using internal nds file names
- can now set the favorites menu to auto show on boot
- can now set an audio file to play on boot (press select on an audio file)

Release 2 - 5:
- added Properties menu which contains info on file types, size and for nds roms, the game ID
- fixed memory corruption in nds internal name caching
- other tiny fixes

Release 2 - 4:
- fixed backgrounding and returning from media player in sub menus
- fixed memory leak in song loading
- added 24 hour clock option
- fixed nds internal names
- fixed the day display on the clock
- covers are now loaded in the favorites menu
- fixed a memory leak with favorites icons
- included a font creation program - dsFont

Release 2 - 3:
- fixed freezing when accessing an empty favorites list
- favorites menu now ignores blank lines in the favorites.txt
- fixed glitch with music playing and the file options menu
- fixed issue with internal nds names while listening to music
- fixed a bug where the stylus couldn't select bottom half of files in directories with few files/folders
- can now use left and right on the d-pad to select cpu frequency in options
- fixed a bug in shortening names with more than one '.'

Release 2 - 2:
- fixed a small memory leak
- added the option to display internal nds file names
- fixed division by 0 error when in favorites menu with exactly 15 favorites
- fixed cursor flickering issue

Release 2 - 1:
- nds covers are now identified using game id's

Release 2:
- increase max files per folder to 1024 and max folders to 64
- fixed a memory leak in skin listing
- fixed menu icons updating on skin changes
- improved file icon handling, common icons are now cached when loaded
- added page scrolling in main browser using left and right on the d-pad
- fixed png support for graphics
- added the option change cpu settings for various menu portions
- improved windowing
- stylus & pad support for all menus
- fixed media player freeze when launching audio from root directory
- added cut, copy, paste actions to filebrowser
- can now add and remove favorites
- changed background music controls (L and R now change tracks)
- added mp3 support to media player
- added ability to have a background for the favorites menu
- added scrolling to the favorites menu
- added ability to enable or disable short names in the favorites menu
- media player is now fully customizable
- added commercial rom support via EOS 1.11
- CATSFC has been added as an alternative choice for snes games
- much faster file sorting
- custom icons can be set to be loaded from a folder on a per skin basis
- can now hide or display file extensions
- fixed various issues with windows and flickering
- added top screen background
- added cover art display on top screen

*Notes*
FAT16 is highly recommended over FAT32 for a considerable speed boost in loading and browsing. The Supercard seems to support FAT16 using 64kb clusters which should cover sd cards up to 2tb.

Release 1:
- initial release
