#!/bin/bash
#

set -eu
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR


# install depend package
sudo apt install libgtk-3-dev libxml2-dev glade librsvg2-bin cmake mingw-w64 -y



