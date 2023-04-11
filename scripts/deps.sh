#!/bin/bash

# current file directory
CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# shellcheck source=./utils/misc.sh
source "$CURRENT_DIR/utils/misc.sh"      # command_exists
# shellcheck source=./utils/xmake.sh
source "$CURRENT_DIR/utils/xmake.sh"     # check_if_repositories_exists_and_add, check_if_dependencies_exists_and_install
# shellcheck source=./utils/constants.sh
source "$CURRENT_DIR/utils/constants.sh" # CONFIG_FILE DEPENDENCIES REPOSITORIES

usage() {
  echo "Usage: $0 [options]"
  echo "Options:"
  echo "  -h, --help"
  echo "      Show this help message and exit"
  echo "  -i, --install"
  echo "      Install all the dependencies listed in the file \`dependencies.toml\`"
  echo "  -I, --emit-include-dirs"
  echo "      Emit all the include directories listed in the file \`dependencies.toml\`"
  echo "  -L, --emit-libraries"
  echo "      Emit all the libraries listed in the file \`dependencies.toml\`"
}

if ! commands_exists "xrepo" "stoml"; then
	exit 1
fi

# If there are no arguments, show usage
if [ $# -eq 0 ]; then
	usage
	exit 0
fi

INSTALL=false
EMIT_INCLUDE_DIRS=false
EMIT_LIBRARIES=false

while [[ $# -gt 0 ]]; do
	key="$1"

	case $key in
	-h | --help)
		usage
		exit 0
		;;
	-i | --install)
    INSTALL=true
		shift
		;;
	-I | --emit-include-dirs)
    EMIT_INCLUDE_DIRS=true
		shift
		;;
	-L | --emit-libraries)
    EMIT_LIBRARIES=true
		shift
		;;
	*)     # unknown option
    usage
    exit 1
		;;
	esac
done

install_dependencies() {
	# Read the root directory file `dependencies.toml` and install all the dependencies
	# listed in the file.

	# List repositories
  check_if_repositories_exists_and_add "$REPOSITORIES" "$CONFIG_FILE"

	# List dependencies
  check_if_dependencies_exists_and_install "$DEPENDENCIES"
}

emit_include_dirs() {
	# Read the root directory file `dependencies.toml` and emit all the include directories
	# listed in the file.
  DEPENDENCIES=$(stoml "$CONFIG_FILE" dependencies)

  # Check if the dir $HOME/.xmake/packages exists
  if [ ! -d "$HOME/.xmake/packages" ]; then
    echo "The directory $HOME/.xmake/packages does not exist. Please run \`xrepo install\` first."
    exit 1
  fi

	for DEPENDENCY in $DEPENDENCIES; do
    # Get the first letter of the dependency name
    FIRST_LETTER=$(echo "$DEPENDENCY" | cut -c 1)
    DEPENDENCY_LIST=$(ls "$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY")

    # TODO: Later get the specified version, but for know get the first one
    DEPENDENCY_VERSION=$(echo "$DEPENDENCY_LIST" | head -n 1)

    DEPENDENCY_HASH=$(ls "$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY/$DEPENDENCY_VERSION")

    echo "$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY/$DEPENDENCY_VERSION/$DEPENDENCY_HASH/include"
	done
}

emit_libraries() {
	# Read the root directory file `dependencies.toml` and emit all the include directories
	# listed in the file.
  DEPENDENCIES=$(stoml "$CONFIG_FILE" dependencies)

	for DEPENDENCY in $DEPENDENCIES; do
    # Get the first letter of the dependency name
    FIRST_LETTER=$(echo "$DEPENDENCY" | cut -c 1)
    DEPENDENCY_LIST=$(ls "$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY")

    # TODO: Later get the specified version, but for know get the first one
    DEPENDENCY_VERSION=$(echo "$DEPENDENCY_LIST" | head -n 1)

    DEPENDENCY_HASH=$(ls "$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY/$DEPENDENCY_VERSION")

    LIB_DIR="$HOME/.xmake/packages/$FIRST_LETTER/$DEPENDENCY/$DEPENDENCY_VERSION/$DEPENDENCY_HASH/lib"

    # List files in the directory
    find "$LIB_DIR" -maxdepth 1 -type f -exec echo {} \;
	done
}

if [ "$INSTALL" = true ]; then
	install_dependencies
fi

if [ "$EMIT_INCLUDE_DIRS" = true ]; then
	emit_include_dirs
fi

if [ "$EMIT_LIBRARIES" = true ]; then
	emit_libraries
fi
