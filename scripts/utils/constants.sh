export CONFIG_FILE="nodepp.config.toml"

REPOSITORIES=$(stoml $CONFIG_FILE repositories)
DEPENDENCIES=$(stoml $CONFIG_FILE dependencies)
export REPOSITORIES
export DEPENDENCIES
