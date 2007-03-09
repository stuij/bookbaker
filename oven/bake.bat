@ECHO OFF

..\gbfs\tools\gbfs text.gbfs text.txt
copy /b ..\src\book.pre.gba + text.gbfs book.gba
del text.gbfs
echo.
echo Your freshly baked ebook is waiting for you in this dir. Watch out!! It's still hot!