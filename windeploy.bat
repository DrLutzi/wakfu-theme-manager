REM requires windeployqt.exe to be in PATH
REM first arg is the source folder, second arg is destination folder
windeployqt.exe %2\wtm.exe
set translationFile=%1\ts\wtm_fr-FR.qm
set translationDest=%2\wtm_fr-FR.qm
set configFile=%1\config\config.json
set configDest=%2\config.json
copy %translationFile% %translationDest%
copy %configFile% %configDest%
if not exist "%2\themes" mkdir %2\themes
pause