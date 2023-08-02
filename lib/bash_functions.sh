# Common functions used by scripts
#  - Depending on bash 3 (or higher) syntax

## -- General shell logging cmds --
function err() {
    local exitcode=$1
    shift
    echo "ERROR: $@" >&2
    exit $exitcode
}

function warn() {
    echo "WARN : $@" >&2
}

function info() {
    if [[ -n "$VERBOSE" ]]; then
        echo "INFO : $@" >&2
    fi
}

## -- Functions for scripts

function root_check_run_with_sudo() {
    # Trick so, program can be run as normal user, will just use "sudo"
    #  call as root_check_run_as_sudo "$@"
    if [ "$EUID" -ne 0 ]; then
        if [ -x $0 ]; then # Directly executable use sudo
            warn "Not root, running with sudo"
            sudo -E "$0" "$@"
            exit $?
        fi
        err 2 "cannot perform sudo run of $0"
        exit 2
    fi
}
