#!/bin/sh

#==================
#    FUNCTIONS    #
#==================

# Ask if user wants to continue, exit if not.
confirm() {
    read -p "Do you want to continue? [y/N]: " c
    if [ "$c" = "" -o "$c" = "n" -o "$c" = "N" ]; then
        exit 1
    elif [ "$c" = "y" -o "$c" = "Y" ]; then
        continue
    else
        printf "Abort.\n"
        exit 1
    fi
}

#==================
#  MAIN PROGRAM   #
#==================

printf "This will install the gimli binary to /usr/local/bin/gimli.\n"
confirm
cp bin/gimli /usr/local/bin/gimli
if [ $? -ne 0 ]; then
    printf "Abort.\n"
    exit 1
fi
python3 -m pip install --upgrade .
