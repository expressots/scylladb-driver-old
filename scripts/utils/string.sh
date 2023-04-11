sanitize_string() {
	# Remove newlines, tabs, leading and trailing spaces and ANSI escape sequences.
	echo "$1" | tr -d '\n' | tr -d '\t' | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' | sed 's/\x1b\[[0-9;]*m//g'
}

remove_all_spaces() {
	echo "$1" | tr -d ' '
}
