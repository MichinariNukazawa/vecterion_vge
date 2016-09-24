set PATH=c:\MinGW\bin;%PATH%

set GTK_DIR=library\gtk+-bundle_3.6.4-20130921_win32
set XML2_DIR=library\libxml2
set PATH=%GTK_DIR%\bin;%PATH%
::set PATH=%XML2_DIR%\bin;%GTK_DIR%\bin;%PATH%;

doskey make = mingw32-make $*
doskey rm = del $*

cmd.exe /f:on /k "chcp 65001"

