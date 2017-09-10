vecterion_vge - Vecterion vector graphics editor -
====

The Vecterion is true vector graphics editor for linux (and other).  

# About
Vecterionは、Linuxに本物のベクタ・グラフィックス・エディタを提供するプロジェクトです。  
ついでにWindows版も作成しています。  

# Detail
Vecterion is not Adobe Illustrator.  

# Screenshot/Image
![vecterion](document/image/vecterion_vge_17.09.png)  

# Project goal
- I makes money for happy life and enjoy develop!
- True vector graphic editor for linux.
- Font editor for the [project daisy bell][pixiv_booth_project_daisy_bell].

# Get Vecterion
get source: `git clone https://github.com/MichianriNukazawa/vecterion_vge`  

[Download for windows(v17.03)](https://github.com/MichinariNukazawa/vecterion_vge/releases/download/v17.03/vecterion_vge-win64-17.03-0c6dd16.zip)  
Download ubuntu package(.deb) @todo  

# Donate/Buy
@todo  
Online store [project daisy bell][pixiv_booth_project_daisy_bell] and [RuneAMN fonts Pro][gumroad_runeamn_fonts_pro] is product by daisy bell.  
And please contact.  

# Running
In Linux. (Ubuntu 16.04LTS+)  

In Windows is [README_win.md](README_win.md).  

## Build
`make`  
`make run`  

## Develop
`bash setup/setup.sh`  
read to [test/release_test.md](test/release_test.md).  

## Depend
External dependency in library/ directory.  
`cd library/`  

### gtk3
https://www.gtk.org/  
`sudo apt install libgtk-3-dev -y`  

### libxml2
http://www.xmlsoft.org/  
`sudo apt install libxml2-dev -y`  

### googletest
https://github.com/google/googletest  
Need for test.  
Make up `libgtest.a libgtest_main.a` in googletest directory.  
```
cd library/
git clone --depth=1 https://github.com/google/googletest.git
cd googletest/googletst/
sudo apt install cmake -y
cmake . && make
```

### rsvg
Need for test.  
`sudo apt install rsvg -y`  
or ubuntu 17.10 later is  
`sudo apt install librsvg2-bin -y`  

### glade
Need for edit custom dialog.  
`sudo apt install glade -y`  

# Specification

## Already implement
- edit bezier curve
- layer, gropu
- rgba color
- import svg file (status of alpha)
- save to svg file (ignore raster image rotation)
- export png,jpg,bmp (raster image) file

import svg file:  
benchmark is [File:Ghostscript Tiger.svg](https://commons.wikimedia.org/wiki/File:Ghostscript_Tiger.svg)?  
 full visible feature and human eye lazy check.  

## Todo
### Short Todo

benchmark is [railmaps](https://github.com/hashcc/railmaps).  

- snap (for pixel, grid, degree, guide line, other element anchor point)
    - SnapForGrid, SnapForDegree is already implement
- BasicShape
    - circle
    - polygon
    - star
- Text element
- group edit mode
- complex Mask Grouping
    - nested
    - color, alpha
- color gradation

- clipboard for system (Ctrl+X,C,V svg, Bitmap to/from other apps)
- confirm dialog when close unsaved document Ctrl+D
- layer naming
- layer thumbnail
- auto save & crash recovery
- appearance
- documentation

### Long Todo
- dark skin ui
- scripting & CLI automation
- auto crash report
- auto update

## License
[LICENSE.md](LICENSE.md)  

# Contact
mail: [michinari.nukazawa@gmail.com][mailto]  
twitter: [@MNukazawa][twitter]  

Develop by Michinari.Nukazawa, in project "daisy bell".  

[pixiv_booth_project_daisy_bell]: https://daisy-bell.booth.pm/
[gumroad_runeamn_fonts_pro]: https://gumroad.com/l/UNWF
[blog_article]: http://blog.michinari-nukazawa.com/
[mailto]: mailto:michinari.nukazawa@gmail.com
[twitter]: https://twitter.com/MNukazawa

