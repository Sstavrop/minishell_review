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


void handle_pipes(t_minishell *ms, t_minishell *commands) 
{
    int num_pipes = commands->pipe_count;
    int pipefd[2];
    pid_t pid;
    t_minishell *current_command = commands;
    int in_fd = 0; //input starts at standard in
    int i = 0;
    int status;
  
    while(i <= num_pipes)
    {
        if (i < num_pipes) //if it is not the last command, make a pipe
        {
            if (create_pipe(pipefd) == -1)
                return ; //create pipe handles printing the error
        }
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)//child
        {
            if (i > 0) //if it is not the first command
            {
                if (dup2_and_close(in_fd, STDIN_FILENO) == -1)
                   exit(EXIT_FAILURE); //dup2 and close will handle error messages
            }
            if (i < num_pipes) // if it is not the last command
            {
                close_fd(pipefd[0]); //close the read end of the pipe
                if(dup2_and_close(pipefd[1], STDOUT_FILENO) == -1)
                   exit(EXIT_FAILURE);
            }
            if (handle_redirections(current_command, ms->heredoc_num) < 0) //handle redirections before execution
                exit(EXIT_FAILURE);
            ms->arguments_tmp = current_command->arguments; //set up arguments for builtins
            if(is_builtin(current_command))
            {
                exec_builtin(ms, current_command); //builtin needs command as arg
                exit(ms->last_exit_status); //exit with builtin status
            }
            else
            {
                execute_external_command(ms, current_command); //external command needs it too
                exit(EXIT_FAILURE); //should not get here on success
            }
        }
        else //parent
        {
            if (i < num_pipes) {
               close_fd(pipefd[1]); // Always close write end in parent
               if (in_fd != 0)  //only close if its not default stdin
                   close_fd(in_fd);   // Close previous read end (if not stdin)
               in_fd = pipefd[0]; // Prepare input for next command
           }
           current_command = current_command->next_command; // Move to next command.
           i++; // Increment the loop counter.
        }
    }
    //wait for all processes
    i = 0;
    while (i <= num_pipes)
    {
        waitpid(-1, &status, WUNTRACED | WCONTINUED);
        if (WIFEXITED(status))
            ms->last_exit_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            ms->last_exit_status = 128 + WTERMSIG(status);
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