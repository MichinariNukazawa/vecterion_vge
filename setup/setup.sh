#!/bin/bash
#

set -eu
set -o pipefail

trap 'echo "error:$0($LINENO) \"$BASH_COMMAND\" \"$@\""' ERR


# install depend package
#sudo apt-get install gtk3-dev libxml2-dev -y

# install git hook
cp ${ROOT_DIR}/develop/pre-commit ${ROOT_DIR}/.git/hook/pre-commit
chmod +x ${ROOT_DIR}/.git/hook/pre-commit

