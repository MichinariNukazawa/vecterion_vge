
# Simple build for windows
Run cross build script.  
`bash deploy/packaging_win64.sh`  

# Using
## MinGW32
http://www.mingw.org/  

## Gtk3 library
http://win32builder.gnome.org/gtk+-bundle_3.6.4-20130921_win32.zip  

extract.  
change  
file "library\gtk+-bundle_3.6.4-20130921_win32\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache"  
"Z:/srv/win32builder/fixed_364/build/win32/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-svg.dll"  
->  
"library/gtk+-bundle_3.6.4-20130921_win32/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-svg.dll"  

## libxml2 library build
http://www.xmlsoft.org/downloads.html  
`git clone git://git.gnome.org/libxml2`  

cd library/libxml2/win32  
cscript configure.js compiler=mingw debug=yes  
make -f Makefile.mingw  
make -f Makefile.mingw install  

