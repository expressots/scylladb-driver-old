log_error() {
	TAG="$(tput bold)$(tput setaf 1)[ERROR]$(tput sgr0)"
	printf "%s: %s%s%s\n" "$TAG" "$(tput setaf 1)" "${1}" "$(tput sgr0)"
}

log_success() {
	TAG="$(tput bold)$(tput setaf 2)[SUCCESS]$(tput sgr0)"
	printf "%s: %s%s%s\n" "$TAG" "$(tput setaf 2)" "${1}" "$(tput sgr0)"
}

log_info() {
	TAG="$(tput bold)$(tput setaf 4)[INFO]$(tput sgr0)"
	printf "%s: %s%s%s\n" "$TAG" "$(tput setaf 4)" "${1}" "$(tput sgr0)"
}

# Exporting: log_error, log_success, log_info
