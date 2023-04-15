#!/bin/bash

# current file directory
CURRENT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

# shellcheck source=./utils/log.sh
source "$CURRENT_DIR/utils/log.sh" # log_error, log_success, log_info
# shellcheck source=./utils/misc.sh
source "$CURRENT_DIR/utils/misc.sh" # command_exists
# shellcheck source=./utils/xmake.sh
source "$CURRENT_DIR/utils/xmake.sh" # check_if_repositories_exists_and_add, check_if_dependencies_exists_and_install
# shellcheck source=./utils/constants.sh
source "$CURRENT_DIR/utils/constants.sh" # DEPENDENCIES REPOSITORIES CONFIG_FILE
# shellcheck source=./utils/patch_shared_libraries.sh
source "$CURRENT_DIR/utils/patch_shared_libraries.sh" # patch_shared_libraries

usage() {
	echo "Usage: $0 [options]"
	echo "Options:"
	echo "  --package-manager yarn|npm"
	echo "  --build"
	echo "  --lsp"
	echo "  --help"
}

PACKAGE_MANAGER="yarn"
BUILD=false
BUILD_RELEASE=false
LSP=false

# If there are no arguments, show usage
if [ $# -eq 0 ]; then
	usage
	exit 0
fi

while [[ $# -gt 0 ]]; do
	key="$1"

	case $key in
	-h | --help)
		usage
		exit 0
		;;
	-pm | --package-manager)
		PACKAGE_MANAGER="$2"
		shift # past argument
		shift # past value
		;;
	-b | --build)
		BUILD=true
		shift # past argument
		;;
	-B | --build-release)
		BUILD_RELEASE=true
		shift # past argument
		;;
	--lsp)
		LSP=true

		# Check if `bear` is installed
		if ! command_exists "bear"; then
			log_error "Please install bear: https://github.com/rizsotto/Bear"
			exit 1
		fi

		shift # past argument
		;;
	*)     # unknown option
		shift # past argument
		;;
	esac
done

# Use yarn or one passed as arguments
if ! command_exists "$PACKAGE_MANAGER"; then
	log_error "Package manager $PACKAGE_MANAGER not found"
	exit 1
fi

# Build the addon
if [ "$BUILD" = true ]; then
	log_info "Building the addon"

	check_if_repositories_exists_and_add "$REPOSITORIES" "$CONFIG_FILE"
	check_if_dependencies_exists_and_install "$DEPENDENCIES"

	if ! $PACKAGE_MANAGER run gen:lib &>/dev/null; then
		log_error "Failed to build the addon"
		exit 1
	fi

	log_success "Addon built"

	if ! command_exists "stoml"; then
		log_error "Please install stoml: https://github.com/freshautomations/stoml"
		exit 1
	fi

	mkdir -p "./node_modules/$(stoml nodepp.config.toml project.name)"
	cp ./build/Release/binding.node "./node_modules/$(stoml nodepp.config.toml project.name)/"

	patch_shared_libraries "./node_modules/$(stoml nodepp.config.toml project.name)/binding.node"
fi

if [ "$LSP" = true ]; then
	# Do not show the output of the command
	log_info "Generating compile_commands.json"
	rm -rf ./compile_commands.json
	bear -- make -j8 -Cbuild -B &>/dev/null
	log_success "compile_commands generated"
fi

if [ "$BUILD_RELEASE" = true ]; then
	# TODO: On build release we should move the necessary files into the equivalent npm structure
	log_error "Not implemented yet"
fi
