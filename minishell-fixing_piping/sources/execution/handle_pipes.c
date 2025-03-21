/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_pipes.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/03 11:56:30 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/06 14:19:34 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Helper function to create a pipe and handle errors.
int create_pipe(int pipe_fds[2]) {
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return -1; // Return -1 on error.
    }
    return 0; // Return 0 on success.
}

// Helper function to close a file descriptor and handle errors.
void close_fd(int fd) {
    if (fd != -1) {
        if (close(fd) == -1) {
            perror("close"); // Still report errors, but don't exit.
        }
    }
}

// Helper function to duplicate a file descriptor and handle errors.
int dup2_and_close(int oldfd, int newfd) {
    if (oldfd != newfd) { // Optimization: Avoid dup2 if they are the same.
        if (dup2(oldfd, newfd) == -1) {
            perror("dup2");
            return -1; // Return -1 on error.
        }
        close_fd(oldfd);
    }
    return 0; // Return 0 on success.
}


// Iterative handle_pipes function
void handle_pipes(t_minishell *ms, t_minishell *commands) {
    int num_pipes = commands->pipe_count;
    int pipefd[2];
    pid_t pid;
    t_minishell *current_command = commands; // Start with the head of the list
    int in_fd = 0; // Initialize input file descriptor to stdin
    int i = 0; //initialize before loop
    int status;

    while (i <= num_pipes) {  // Corrected loop condition

        if (i < num_pipes) {
            if (create_pipe(pipefd) == -1) {
                return; // Error already printed
            }
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            // Input Redirection (except for the first command)
            if (i > 0) {
              if(dup2_and_close(in_fd, STDIN_FILENO) == -1) //check for errors
                exit(EXIT_FAILURE);
            }

            // Output Redirection (except for the last command)
            if (i < num_pipes) {
                close_fd(pipefd[0]); // Close read end in child.
                if(dup2_and_close(pipefd[1], STDOUT_FILENO) == -1) //check for errors
                  exit(EXIT_FAILURE);
            }
            //handle redirections before execution
            if (handle_redirections(current_command, ms->heredoc_num) < 0)
                exit(EXIT_FAILURE);
            ms->arguments_tmp = current_command->arguments; //needed for builtins
            if (is_builtin(current_command)) { //pass the command
                exec_builtin(ms, current_command); // Pass current_command
                exit(ms->last_exit_status);
            } else {
                execute_external_command(ms, current_command); //pass in current command
                exit(EXIT_FAILURE); // Should never reach here if successful
            }
        } else { // Parent process
            close_fd(pipefd[1]); // Always close write end in parent
            if (in_fd != 0)
                close_fd(in_fd);   // Close previous read end (if not stdin)
            in_fd = pipefd[0]; // Prepare input for the next command

            current_command = current_command->next_command; // Move to next command
            i++; // Increment the loop counter.
        }
    }
     // Wait for *all* child processes to finish, in the parent.
    i = 0;
    while(i <= num_pipes)
    {
        waitpid(-1, &status, WUNTRACED | WCONTINUED); // Corrected waitpid call
        if (WIFEXITED(status)) {
            ms->last_exit_status = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            ms->last_exit_status = 128 + WTERMSIG(status); // Standard convention
        }
        i++;
    }
}

int contains_pipe(t_minishell *commands) { //simpler check
    t_minishell *current = commands;
    while (current) {
        if (current->operator == PIPE) {
            return 1;
        }
        current = current->next_command;
    }
    return 0;
}