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

int	contains_pipe(t_minishell *commands)
{
    t_minishell	*current;

    current = commands;
    while (current)
    {
        if (current->type == T_PIPE)
            return (1);
        current = current->next;
    }
    return (0);
}

int	count_pipes(t_minishell *commands)
{
	int			count;
	t_minishell	*current;

	count = 0;
	current = commands;
	while (current)
	{
		if (current->type == T_PIPE)
			count++;
		current = current->next;
	}
	return (count);
}

void	child_process(int *pipefd, t_minishell *ms, t_minishell *cmd_list,
		int prev_fd)
{
	if (prev_fd != -1)
	{
		if (dup2(prev_fd, STDIN_FILENO) == -1) // Use previous pipe as input
		{
			perror("dup2 (prev_fd to STDIN)\n");
			exit(EXIT_FAILURE);
		}
		close(prev_fd);
	}
	if (cmd_list->next_command)
	{
		close(pipefd[0]);               // Close unused read end
		if (dup2(pipefd[1], STDOUT_FILENO) == -1)
		{
			perror("dup2 (pipefd[1] to STDOUT)\n");
			exit(EXIT_FAILURE);
		}
		close(pipefd[1]);
	}
	else
	{
		close(pipefd[0]);
		close(pipefd[1]);
	}
	if (handle_redirections(cmd_list, ms->heredoc_num) < 0)
		exit(EXIT_FAILURE);
	ms->arguments_tmp = cmd_list->arguments;
	if (is_builtin(ms))
		exec_builtin(ms);
	else
		execute_external_command(ms, cmd_list);
	exit(EXIT_SUCCESS);
}

void	parent_process(int *pipefd, t_minishell *cmd_list, int *prev_fd)
{
	if (*prev_fd != -1)
		close(*prev_fd); // Close the previous read end
	if (cmd_list->next_command)
	{
		close(pipefd[1]);     // Close current write end
		*prev_fd = pipefd[0]; // Save read end for the next command
	}
	else
	{
		close(pipefd[0]);
		close(pipefd[1]);
	}
}

void	handle_pipe(t_minishell *ms, t_minishell *cmd_list, int prev_fd,
		int num_of_pipes)
{
	int pipefd[2];
	int saved_stdin;
	int saved_stdout;
	pid_t pid;

	saved_stdin = dup(STDIN_FILENO);
	saved_stdout = dup(STDOUT_FILENO);
	if (num_of_pipes > 0 && pipe(pipefd) == -1)
	{
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (pid == 0)
		child_process(pipefd, ms, cmd_list, prev_fd);
	else
	{
		parent_process(pipefd, cmd_list, &prev_fd);
		waitpid(pid, NULL, 0);
	}
	if (num_of_pipes == 0)
	{
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdin);
		close(saved_stdout);
	}
	else
		handle_pipe(ms, cmd_list->next_command, prev_fd, num_of_pipes - 1);
}
