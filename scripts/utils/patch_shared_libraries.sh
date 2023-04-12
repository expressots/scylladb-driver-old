#!/bin/bash

# current file directory
CURRENT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

# shellcheck source=./misc.sh
source "$CURRENT_DIR/misc.sh" # command_exists
# shellcheck source=./constants.sh
source "$CURRENT_DIR/constants.sh" # CONFIG_FILE

patch_shared_libraries() {
	if ! command_exists patchelf; then
		echo "patchelf not found, please install it first"
		exit 1
	fi

	ELF_FILE="$1"

	DEPENDENCIES=$(stoml "$CONFIG_FILE" dependencies)

	for DEPENDENCY in $DEPENDENCIES; do
		# Get the first letter of the dependency name
		FIRST_LETTER=$(echo "$DEPENDENCY" | cut -c 1)
		DEPENDENCY_LIST=$(ls "$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY")

		# TODO: Later get the specified version, but for know get the first one
		DEPENDENCY_VERSION=$(echo "$DEPENDENCY_LIST" | head -n 1)

		DEPENDENCY_HASH=$(ls "$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY/$DEPENDENCY_VERSION")

		LIB_DIR="$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY/$DEPENDENCY_VERSION/$DEPENDENCY_HASH/lib"

		# If the file is a shared library, add it to the list
		# find "$LIB_DIR" -maxdepth 1 -type f -exec echo {} \;
		find "$LIB_DIR" -maxdepth 1 -type f -exec file {} \; | grep "shared object" | cut -d: -f1 | while read -r SHARED_LIB; do
			patchelf --add-needed "$SHARED_LIB" "$ELF_FILE"
		done
	done
}