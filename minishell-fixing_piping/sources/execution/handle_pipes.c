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
#include <errno.h>

int create_pipe(int pipe_fds[2]) 
{
    if (pipe(pipe_fds) == -1) 
    {
        perror("pipe");
        return (-1);
    }
    return (0);
}

void close_fd(int fd)
{
    if (fd != -1)
    {
        if (close(fd) == -1)
            perror("close");
    }
}

int dup2_and_close(int oldfd, int newfd) 
{
    if (oldfd != newfd) 
    {
        if (dup2(oldfd, newfd) == -1) 
        {
            perror("dup2");
            return (-1);
        }
        close_fd(oldfd);
    }
    return (0);
}

int contains_pipe(t_minishell *commands)
{
    t_minishell *current = commands;
    while (current) 
    {
        if (current->operator == T_PIPE) 
        return (1);
        current = current->next_command;
    }
    return (0);
}

int calculate_num_pipes(t_minishell *commands) 
{
    int count;
    t_minishell *cmd;
    
    count = 0;
    cmd = commands;
    while (cmd != NULL && cmd->next_command != NULL) 
    {
        if (cmd->operator == T_PIPE) 
        count++;
        cmd = cmd->next_command;
    }
    return (count);
}

// #include "minishell.h" // Ensure this includes stdio.h, unistd.h, sys/wait.h etc.
// #include <stdio.h>     // Explicitly include for fprintf, perror
// #include <unistd.h>    // Explicitly include for getpid, fork, close, dup2, etc.
// #include <sys/wait.h>  // Explicitly include for waitpid, macros
// #include <string.h>    // Explicitly include for strerror (if using instead of perror)
// #include <errno.h>     // Explicitly include for errno
// #include <stdlib.h>    // Explicitly include for malloc, free, exit

// Assumes calculate_num_pipes, create_pipe, close_fd, handle_redirections,
// is_builtin, exec_builtin, execute_external_command, add_token, free_token_list,
// free_array, pathfinder, convert_env_to_array, handle_exece_failure, isatty
// exist and have correct prototypes in minishell.h

// Assuming close_fd is a simple wrapper like:
// void close_fd(int fd) { if (fd > STDERR_FILENO) close(fd); } // Avoid closing 0,1,2 by mistake

void handle_pipes(t_minishell *ms, t_minishell *commands)
{
    int num_pipes;
    int pipefd[2] = {-1, -1}; // Initialize pipefds
    pid_t pid;
    t_minishell *current_command;
    int in_fd; // Represents the FD the current command should read from
    int i;
    int status;
    pid_t *child_pids = NULL; // To store PIDs

    num_pipes = calculate_num_pipes(commands); // Ensure this uses T_PIPE!
    current_command = commands;
    in_fd = STDIN_FILENO; // First command reads from standard input
    i = 0; // Command index

    // Allocate array to store child PIDs (+1 for N+1 commands)
    child_pids = malloc(sizeof(pid_t) * (num_pipes + 1));
    if (!child_pids) {
        fprintf(stderr,"minishell: malloc failed for pid array\n");
        ms->last_exit_status = 1;
        return;
    }
    // Initialize PIDs array
    for (int k = 0; k <= num_pipes; k++) {
        child_pids[k] = -1; // Initialize PIDs to an invalid value
    }

    fprintf(stderr, "[DEBUG] Parent (pid %d): Starting handle_pipes. num_pipes=%d\n", getpid(), num_pipes);

    // --- Main loop to create processes for each command ---
    while (i <= num_pipes) // Loop N+1 times for N pipes (N+1 commands)
    {
        fprintf(stderr, "[DEBUG] Parent (pid %d): TOP OF LOOP - i=%d\n", getpid(), i);

        // 1. Create pipe if needed (for all except the last command)
        if (i < num_pipes) {
            // Using create_pipe helper for consistency
            if (create_pipe(pipefd) == -1) {
                ms->last_exit_status = 1;
                free(child_pids);
                // TODO: Need robust cleanup of previously forked children
                return;
            }
            fprintf(stderr, "[DEBUG] Parent (pid %d): Created pipe for cmd %d: read_fd=%d, write_fd=%d\n", getpid(), i, pipefd[0], pipefd[1]);
        }

        // 2. Fork
        pid = fork();
        if (pid == -1) { // Fork Failed
            perror("minishell: fork");
            ms->last_exit_status = 1;
            // Close pipe ends if they were just created for this iteration
            if (i < num_pipes) { close_fd(pipefd[0]); close_fd(pipefd[1]); }
            // Close previous input fd if it's a pipe
            if (in_fd != STDIN_FILENO) close_fd(in_fd);
            free(child_pids);
            // TODO: Need robust cleanup of previously forked children
            break; // Exit loop on fork failure
        }

        // --- Child Process ---
        if (pid == 0)
        {
            free(child_pids); // Child doesn't need PID array
            fprintf(stderr, "[DEBUG] Child (cmd %d, pid %d): STARTING\n", i, getpid());

            // 3a. Redirect Input (from previous pipe)
            if (i > 0) { // If it is not the first command
                fprintf(stderr, "[DEBUG] Child (cmd %d): Dup2 STDIN from %d\n", i, in_fd);
                if (dup2(in_fd, STDIN_FILENO) == -1) { perror("Child stdin dup2"); exit(1); }
                fprintf(stderr, "[DEBUG] Child (cmd %d): Closing original in_fd %d\n", i, in_fd);
                if (close(in_fd) == -1) { perror("Child close in_fd"); exit(1); }
            }
            // 3b. Redirect Output (to next pipe)
            if (i < num_pipes) { // If it is not the last command
                fprintf(stderr, "[DEBUG] Child (cmd %d): Closing pipe READ end %d\n", i, pipefd[0]);
                if (close(pipefd[0]) == -1) { perror("Child close pipefd[0]"); exit(1); } // Use raw close

                fprintf(stderr, "[DEBUG] Child (cmd %d): Dup2 STDOUT to pipe WRITE end %d\n", i, pipefd[1]);
                if (dup2(pipefd[1], STDOUT_FILENO) == -1) { perror("Child stdout dup2"); exit(1); }

                fprintf(stderr, "[DEBUG] Child (cmd %d): Closing original pipe WRITE end %d\n", i, pipefd[1]);
                if (close(pipefd[1]) == -1) { perror("Child close pipefd[1]"); exit(1); } // Use raw close
            }

            // 3c. Handle File Redirections (AFTER pipe setup)
            fprintf(stderr, "[DEBUG] Child (cmd %d): Handling file redirections...\n", i);
            if (handle_redirections(current_command, ms->heredoc_num) < 0) {
                 // Assuming handle_redirections prints error and sets status
                 exit(ms->last_exit_status);
            }

            // 4. Execute
            ms->arguments_tmp = current_command->arguments; // Bridge for old builtins
            fprintf(stderr, "[DEBUG] Child (cmd %d): Executing [%s]...\n", i, current_command->arguments ? current_command->arguments[0] : "NULL_CMD");
            if (is_builtin(current_command)) {
                exec_builtin(ms, current_command);
                exit(ms->last_exit_status); // Builtin sets its own status
            } else {
                execute_external_command(ms, current_command); // This calls execve
                // If execve fails, execute_external_command should exit with 126/127
                // This exit is a fallback
                exit(127);
            }
        }
        // --- Parent Process ---
        else
        {
            child_pids[i] = pid; // Store PID of the child we just forked
            fprintf(stderr, "[DEBUG] Parent (pid %d): Forked child %d (PID %d) for cmd %d\n",
                    getpid(), i, pid, i);

            // --- Parent Pipe FD handling ---
            // Parent closes FDs it no longer needs for communication

            // Close the read end from the *previous* pipe first.
            if (i > 0) { // Or more safely: if (in_fd != STDIN_FILENO)
                fprintf(stderr, "[DEBUG] Parent (pid %d): Closing previous in_fd=%d\n", getpid(), in_fd);
                close_fd(in_fd); // Using close_fd helper
            }

            // Decide what to do based on whether this is the last command
            if (i < num_pipes) {
                 // ---- This is NOT the last command ----
                 // Close the write end of the pipe we just created (child uses it)
                 fprintf(stderr, "[DEBUG] Parent (pid %d): Closing pipe WRITE end fd %d\n", getpid(), pipefd[1]);
                 close_fd(pipefd[1]);
                 // Save the read end for the *next* child
                 in_fd = pipefd[0];
                 fprintf(stderr, "[DEBUG] Parent (pid %d): Saved pipe READ end fd %d as in_fd\n", getpid(), in_fd);
            }
            else {
                 // ---- This IS the iteration for the LAST command ----
                 // No new pipe was created.
                 // The previous in_fd (if it existed) was closed in the 'if (i > 0)' block above.
                 // Reset in_fd so the close *after* the loop doesn't run on the old FD.
                 fprintf(stderr, "[DEBUG] Parent (pid %d): Resetting in_fd for last cmd %d\n", getpid(), i);
                 in_fd = STDIN_FILENO; // Reset to default/non-pipe value
            }
            // --- End Parent Pipe FD handling ---

            // --- Advance to next command ---
            if (current_command) // Check before accessing next_command
                 current_command = current_command->next_command;
            else
                 fprintf(stderr, "[DEBUG] Parent: ERROR current_command was NULL before trying to advance!\n");

            i++;
            fprintf(stderr, "[DEBUG] Parent: Incremented i to %d\n", i);
            // --- End Advance ---
        }
        fprintf(stderr, "[DEBUG] Parent: BOTTOM OF LOOP - i=%d, num_pipes=%d\n", i, num_pipes);
    } // End while loop

    fprintf(stderr, "[DEBUG] Parent: EXITED LOOP - i=%d, num_pipes=%d\n", i, num_pipes);

    // --- After loop: Parent potentially closes the final read end ---
    // This check should now be FALSE if pipes existed because in_fd was reset.
    if (in_fd != STDIN_FILENO) {
        fprintf(stderr,"[DEBUG] Parent (pid %d): Closing FINAL in_fd=%d (Should be STDIN if pipes were used!)\n", getpid(), in_fd);
        close_fd(in_fd);
    }

    // --- Wait for all children ---
    fprintf(stderr,"[DEBUG] Parent (pid %d): Starting wait loop...\n", getpid());
    i = 0;
    while (i <= num_pipes) { // Wait for N+1 children
        pid_t waited_pid = waitpid(-1, &status, 0);
        if (waited_pid < 0) {
             if (errno == ECHILD) {
                  fprintf(stderr,"[DEBUG] Parent: No more children to wait for (ECHILD).\n");
                 break;
             }
             fprintf(stderr,"minishell: waitpid error in handle_pipes (errno %d)\n", errno);
             ms->last_exit_status = 1; // Set error status
             break;
        }

        // Find which command index this PID corresponds to
        int cmd_index = -1;
        for(int j=0; j <= num_pipes; ++j) {
            if(child_pids[j] == waited_pid) {
                cmd_index = j;
                child_pids[j] = -1; // Mark as reaped
                break;
            }
        }
        fprintf(stderr,"[DEBUG] Parent: Reaped child PID %d (cmd %d)\n", waited_pid, cmd_index);

        // Set status ONLY from the LAST command in the pipeline (index num_pipes)
        if (cmd_index == num_pipes) {
             if (WIFEXITED(status)) {
                 ms->last_exit_status = WEXITSTATUS(status);
             } else if (WIFSIGNALED(status)) {
                 ms->last_exit_status = 128 + WTERMSIG(status);
                  if (isatty(STDERR_FILENO)) { // Optional: Print signal info only if terminal
                      if (WTERMSIG(status) == SIGINT) fprintf(stderr, "\n");
                      if (WTERMSIG(status) == SIGQUIT) fprintf(stderr, "Quit: %d\n", WTERMSIG(status));
                  }
             }
             fprintf(stderr,"[DEBUG] Parent: Set last_exit_status=%d from last cmd (%d)\n", ms->last_exit_status, cmd_index);
        }
        i++;
    }
    fprintf(stderr,"[DEBUG] Parent (pid %d): Finished wait loop. Final status = %d\n", getpid(), ms->last_exit_status);
    free(child_pids); // Free the PID array
}