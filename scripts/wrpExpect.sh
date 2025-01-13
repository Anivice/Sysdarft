#!/usr/bin/expect -f

# Disable output buffering
log_user 1

# Check if a command is provided
if { $argc < 1 } {
    puts stderr "Usage: run_with_expect.exp <command> [args...]"
    exit 1
}

# Get command and arguments
set cmd [lindex $argv 0]
set args [lrange $argv 1 end]

# Spawn the command
spawn {*}$argv

# Wait for the command to finish
expect eof

# Get the exit status
set exit_status [wait]
set code [lindex $exit_status 3]

# Exit with the command's exit code
exit $code
