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

void handle_pipes(t_minishell *ms, t_minishell *commands) 
{
    int num_pipes;
    int pipefd[2];
    pid_t pid;
    t_minishell *current_command;
    int in_fd;
    int i;
    int status;
    int fd_to_close;

    num_pipes = calculate_num_pipes(commands);
    current_command = commands;
    in_fd = STDIN_FILENO;
    i = 0;
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
        if (pid == 0)//child block
        {
            if (i > 0) //if it is not the first command
            {
                printf("Child (cmd %d): Attempting dup2_and_close(in_fd=%d, STDIN_FILENO)\n", i, in_fd);
                fd_to_close = in_fd; // Save value before dup2_and_close closes it
                if (dup2_and_close(in_fd, STDIN_FILENO) == -1)
                    exit(EXIT_FAILURE); //dup2 and close will handle error messages
                printf("Child (cmd %d): Closed read end from prev pipe: %d\n", i, in_fd); 
            }
            if (i < num_pipes) // if it is not the last command
            {
                printf("Child (cmd %d): Closing unused READ end pipefd[0]=%d\n", i, pipefd[0]);
                close_fd(pipefd[0]); //close the read end of the pipe
                printf("Child (cmd %d): Attempting dup2_and_close(pipefd[1]=%d, STDOUT_FILENO)\n", i, pipefd[1]);
                fd_to_close = pipefd[1];
                if(dup2_and_close(pipefd[1], STDOUT_FILENO) == -1)
                   exit(EXIT_FAILURE);
                printf("Child (cmd %d): Ran dup2_and_close for pipefd[1]=%d -> STDOUT\n", i, fd_to_close); 
            }
            printf("Child (cmd %d): About to execute [%s]...\n", i, current_command->arguments[0]);
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
        else //parent block
        {
            if (i < num_pipes)
            {
               close_fd(pipefd[1]); // Always close write end in parent
               printf("Parent: Closed pipefd[1]=%d for cmd %d\n", pipefd[1], i); // Debug close
               if (in_fd != STDIN_FILENO)  //only close if its not default stdin
               {
                   close_fd(in_fd);   // Close previous read end (if not stdin)
                   printf("Parent: Closed previous in_fd=%d for cmd %d\n", in_fd, i); // Debug close
               }
               in_fd = pipefd[0]; // Prepare input for next command
           }
        }
        if (pid > 0)
        {
            current_command = current_command->next_command; // Move to next command.
            i++; // Increment the loop counter.
        }
        else if (pid < 0)
            break;
    }
    if (in_fd != STDIN_FILENO)
    {
        close_fd(in_fd);
        printf("Parent: Closed FINAL in_fd=%d\n", in_fd);
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

int contains_pipe(t_minishell *commands)
{
    t_minishell *current = commands;
    while (current) 
    {
        if (current->operator == PIPE) 
            return (1);
        current = current->next_command;
    }
    return (0);
}
