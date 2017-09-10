vecterion_vge build for windows
====

## mingw-64
`sudo apt install mingw-w64 -y`  

## gtk3 prebuild binary for windows x64
automatically by build script.  

### update gtk3 prebuild binary
Download and convert gtk3 library.  
gtk library manual downloading (please read that script.)  
`deploy/win/gtk_library/gtk_library_repackage.sh`  

## libxml2 library
`git clone git://git.gnome.org/libxml2`  
path for `library/libxml2/`  

## build and packaging
Run cross build script.  
`bash deploy/packaging_win64.sh`  

