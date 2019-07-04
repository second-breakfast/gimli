#!/bin/sh

#==================
#    FUNCTIONS    #
#==================

# Ask if user wants to continue, exit if not.
confirm() {
    read -p "Continue? (y/N): " c
    if [ "$c" = "" -o "$c" = "n" -o "$c" = "N" ]; then
        exit 1
    elif [ "$c" = "y" -o "$c" = "Y" ]; then
        continue
    else
        printf "Invalid response, exiting...\n"
        exit 1
    fi
}

#==================
#  MAIN PROGRAM   #
#==================

# Install gimli via the local PyPi package in this directory.
pip3 install --upgrade .

# Copy gimli binary to $HOME/bin (ask to create $HOME/bin if it doesn't exist).
if [ -d $HOME/bin ]; then
    cp bin/gimli $HOME/bin/
else
    printf "\nNo bin folder at $HOME/bin. Would you like to create it?\n"
    confirm
    mkdir $HOME/bin > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        printf "error: Unable to create $HOME/bin\n"
        exit 1
    fi
    cp bin/gimli $HOME/bin/
fi

printf "\nThe gimli binary has been installed to $HOME/bin/gimli.\n"
printf "\nTo run gimli, do:\n\n"
printf "  export PATH=\$PATH:\$HOME/bin\n  gimli -h\n\n"
