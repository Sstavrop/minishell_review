/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute_builtins.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 14:06:34 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/06 14:19:45 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	exec_builtin(t_minishell *ms)
{
	if (!ft_strncmp("exit", ms->arguments_tmp[0], 5))
		ft_exit(ms);
	else if (!ft_strncmp("echo", ms->arguments_tmp[0], 5))
		ft_echo(ms);
	else if (!ft_strncmp("env", ms->arguments_tmp[0], 4))
		ft_env(ms);
	else if (!ft_strncmp("pwd", ms->arguments_tmp[0], 4))
		ft_pwd();
	else if (!ft_strncmp("export", ms->arguments_tmp[0], 7))
		ft_export(ms);
	else if (!ft_strncmp("unset", ms->arguments_tmp[0], 6))
		ft_unset(ms);
	else if (!ft_strncmp("cd", ms->arguments_tmp[0], 3))
		ft_cd(ms);
}

int	is_builtin(t_minishell *ms)
{
	int	trigger;

	trigger = 0;
	if (ms->arguments_tmp == NULL || ms->arguments_tmp[0] == NULL)
		return (0);
	if (!ft_strncmp("exit", ms->arguments_tmp[0], 5))
		trigger = 2;
	else if (!ft_strncmp("echo", ms->arguments_tmp[0], 5))
		trigger = 1;
	else if (!ft_strncmp("env", ms->arguments_tmp[0], 4))
		trigger = 1;
	else if (!ft_strncmp("pwd", ms->arguments_tmp[0], 4))
		trigger = 1;
	else if (!ft_strncmp("export", ms->arguments_tmp[0], 7))
		trigger = 1;
	else if (!ft_strncmp("unset", ms->arguments_tmp[0], 6))
		trigger = 1;
	else if (!ft_strncmp("cd", ms->arguments_tmp[0], 3))
		trigger = 2;
	return (trigger);
}

void	execute_piped_commands(t_minishell *commands, t_minishell *ms)
{
	int	num_of_pipes;

	num_of_pipes = count_pipes(commands);
	handle_pipe(ms, commands, -1, num_of_pipes);
}

void	execute_commands_loop(t_minishell *ms, t_minishell *commands,
		int heredoc_num)
{
	t_minishell	*current;

	current = commands;
	while (current)
	{
		ms->arguments_tmp = current->arguments;
		if (handle_redirections(current, heredoc_num) < 0)
		{
			current = current->next;
			continue ;
		}
		ms->arguments_tmp = current->arguments;
		if (is_builtin(ms))
			exec_builtin(ms);
		else
			execute_external_command(ms, current);
		dup2(ms->ter_in, STDIN_FILENO);
		dup2(ms->ter_out, STDOUT_FILENO);
		current = current->next;
	}
}

void	execute_command(t_minishell *ms, t_minishell *token_list,
		int heredoc_num)
{
	t_minishell	*commands;

	if (!token_list)
	{
		ft_printf("Error: No tokens to execute.\n");
		return ;
	}
	commands = parse_tokens_into_commands(token_list);
	if (!commands)
	{
		ft_printf("Error: Parsing tokens into commands.\n");
		return ;
	}
	ms->heredoc_num = heredoc_num;
	if (contains_pipe(commands))
	{
		// printf("DEBUG: Entering piping execute\n");
		execute_piped_commands(commands, ms);
	}
	else
	{
		// printf("DEBUG: Entering regular execute\n");
		execute_commands_loop(ms, commands, heredoc_num);
	}
	free_command(commands);
}
