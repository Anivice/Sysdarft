#include <cstdint>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

/* Since pipes are unidirectional, we need three pipes:
   1. Parent writes to child's stdin
   2. Child writes to parent's stdout
   3. Child writes to parent's stderr
*/

#define NUM_PIPES           3

#define PARENT_WRITE_PIPE   0
#define PARENT_READ_PIPE    1
#define PARENT_ERR_PIPE     2

int pipes[NUM_PIPES][2];

/* Always in a pipe[], pipe[0] is for read and
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1

#define PARENT_READ_FD   ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD  ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )
#define PARENT_ERR_FD    ( pipes[PARENT_ERR_PIPE][READ_FD]    )

#define CHILD_READ_FD    ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD   ( pipes[PARENT_READ_PIPE][WRITE_FD]  )
#define CHILD_ERR_FD     ( pipes[PARENT_ERR_PIPE][WRITE_FD]   )

// Define the cmd_status struct
struct cmd_status {
    std::string fd_stdout;
    std::string fd_stderr;
    int exit_status;
};

// Helper function to set a string with the current errno message
std::string get_errno_message(const std::string &prefix = "") {
    return prefix + std::strerror(errno);
}

cmd_status _exec_command(
    const std::string &cmd, const std::vector<std::string> &args, const std::string &input) {

    cmd_status status = {"", "", 1}; // Default to failure

    // Initialize all required pipes
    for (int i = 0; i < NUM_PIPES; ++i) {
        if (pipe(pipes[i]) == -1) {
            status.fd_stderr += get_errno_message("pipe() failed: ");
            status.exit_status = 1;
            // TODO: Handle pipe creation failure appropriately
            return status;
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        // Fork failed
        status.fd_stderr += get_errno_message("fork() failed: ");
        status.exit_status = 1;
        // TODO: Handle fork failure appropriately
        // Close all pipes before returning
        for (int i = 0; i < NUM_PIPES; ++i) {
            close(pipes[i][READ_FD]);
            close(pipes[i][WRITE_FD]);
        }
        return status;
    }

    if (pid == 0) {
        // Child process

        // Redirect stdin
        if (dup2(CHILD_READ_FD, STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            exit(EXIT_FAILURE);
        }

        // Redirect stdout
        if (dup2(CHILD_WRITE_FD, STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            exit(EXIT_FAILURE);
        }

        // Redirect stderr
        if (dup2(CHILD_ERR_FD, STDERR_FILENO) == -1) {
            perror("dup2 stderr");
            exit(EXIT_FAILURE);
        }

        /* Close all pipe fds in the child */
        for (int i = 0; i < NUM_PIPES; i++) {
            close(pipes[i][READ_FD]);
            close(pipes[i][WRITE_FD]);
        }

        // Build argv for execv
        std::vector<char *> argv;
        argv.push_back(const_cast<char *>(cmd.c_str()));
        for (const auto &arg : args) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execv(cmd.c_str(), argv.data());

        // If execv fails
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        // Close unused pipe ends in the parent
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(CHILD_ERR_FD);

        // Set the write end of the stdin pipe to non-blocking to handle potential write errors
        // fcntl(PARENT_WRITE_FD, F_SETFL, O_NONBLOCK); // Optional: Depending on requirements

        // Write to child's stdin
        ssize_t total_written = 0;
        ssize_t input_size = input.size();
        const char *input_cstr = input.c_str();
        ssize_t bytes_to_write = input_size;

        // Ensure input ends with a newline
        std::string modified_input = input;
        if (modified_input.empty() || modified_input.back() != '\n') {
            modified_input += "\n";
            input_cstr = modified_input.c_str();
            bytes_to_write = modified_input.size();
        }

        while (total_written < bytes_to_write) {
            ssize_t written = write(PARENT_WRITE_FD, input_cstr + total_written, bytes_to_write - total_written);
            if (written == -1) {
                if (errno == EINTR)
                    continue; // Retry on interrupt
                else {
                    status.fd_stderr += get_errno_message("write() to child stdin failed: ");
                    status.exit_status = 1;
                    // TODO: Decide whether to terminate or continue
                    break;
                }
            }
            total_written += written;
        }

        // Optionally close the write end if no more input is sent
        if (close(PARENT_WRITE_FD) == -1) {
            status.fd_stderr += get_errno_message("close() PARENT_WRITE_FD failed: ");
            // TODO: Handle close failure if necessary
        }

        // Function to read all data from a file descriptor
        auto read_all = [&](int fd, std::string &output) -> bool {
            char buffer[4096];
            ssize_t count;
            while ((count = read(fd, buffer, sizeof(buffer))) > 0) {
                output.append(buffer, count);
            }
            if (count == -1) {
                output += get_errno_message("read() failed: ");
                return false;
            }
            return true;
        };

        // Read from child's stdout
        if (!read_all(PARENT_READ_FD, status.fd_stdout)) {
            status.exit_status = 1;
            // TODO: Handle read stdout failure if necessary
        }

        // Read from child's stderr
        if (!read_all(PARENT_ERR_FD, status.fd_stderr)) {
            status.exit_status = 1;
            // TODO: Handle read stderr failure if necessary
        }

        // Close the read ends
        if (close(PARENT_READ_FD) == -1) {
            status.fd_stderr += get_errno_message("close() PARENT_READ_FD failed: ");
            // TODO: Handle close failure if necessary
        }
        if (close(PARENT_ERR_FD) == -1) {
            status.fd_stderr += get_errno_message("close() PARENT_ERR_FD failed: ");
            // TODO: Handle close failure if necessary
        }

        // Wait for child process to finish
        int wstatus;
        if (waitpid(pid, &wstatus, 0) == -1) {
            status.fd_stderr += get_errno_message("waitpid() failed: ");
            status.exit_status = 1;
            // TODO: Handle waitpid failure if necessary
        } else {
            if (WIFEXITED(wstatus)) {
                status.exit_status = WEXITSTATUS(wstatus);
            } else if (WIFSIGNALED(wstatus)) {
                std::ostringstream oss;
                oss << "Child terminated by signal " << WTERMSIG(wstatus) << "\n";
                status.fd_stderr += oss.str();
                status.exit_status = 1;
            } else {
                // Other cases like stopped or continued
                status.fd_stderr += "Child process ended abnormally.\n";
                status.exit_status = 1;
            }
        }

        return status;
    }
}

int main() {
    cmd_status result = _exec_command("/usr/bin/bc", { }, "255");
    printf("Exit Status: %d\n", result.exit_status);
    printf("Stdout: %s\n", result.fd_stdout.c_str());
    if (!result.fd_stderr.empty()) {
        fprintf(stderr, "Stderr: %s\n", result.fd_stderr.c_str());
    }
    return 0;
}
