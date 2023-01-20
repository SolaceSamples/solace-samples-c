

script_dir="$(dirname -- "$(readlink -f -- "$0")")"
export LD_LIBRARY_PATH=$script_dir
