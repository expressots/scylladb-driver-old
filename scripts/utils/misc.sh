command_exists() {
  # TODO: Check if `type` is generally available
	type "$1" &>/dev/null
}

commands_exists() {
  for cmd in "$@"; do
    command_exists "$cmd" || {
      printf "Command not found: %s. Aborting." "$cmd"
      exit 1
    }
  done
}

# Exporting: command_exists, commands_exists
