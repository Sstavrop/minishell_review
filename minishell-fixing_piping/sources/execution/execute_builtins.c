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

void exec_builtin(t_minishell *ms, t_minishell *command) { //definition
    if (!command || !command->arguments || !command->arguments[0])
        return; // Or handle the error appropriately

    if (ft_strncmp(command->arguments[0], "exit", 5) == 0) {
        ft_exit(ms, command);
    } else if (ft_strncmp(command->arguments[0], "cd", 3) == 0) {
        ft_cd(ms);
    } else if (ft_strncmp(command->arguments[0], "pwd", 4) == 0) {
        ft_pwd(ms);
    } else if (ft_strncmp(command->arguments[0], "echo", 5) == 0) {
       ft_echo(ms, command);
    }
     else if (ft_strncmp(command->arguments[0], "env", 4) == 0) {
        ft_env(ms, command);
    }
      else if (ft_strncmp(command->arguments[0], "export", 7) == 0) {
        ft_export(ms);
    }
      else if (ft_strncmp(command->arguments[0], "unset", 6) == 0) {
        ft_unset(ms);
    }
}
// void	exec_builtin(t_minishell *ms)
// {
// 	if (!ft_strncmp("exit", ms->arguments_tmp[0], 5))
// 		ft_exit(ms);
// 	else if (!ft_strncmp("echo", ms->arguments_tmp[0], 5))
// 		ft_echo(ms);
// 	else if (!ft_strncmp("env", ms->arguments_tmp[0], 4))
// 		ft_env(ms);
// 	else if (!ft_strncmp("pwd", ms->arguments_tmp[0], 4))
// 		ft_pwd();
// 	else if (!ft_strncmp("export", ms->arguments_tmp[0], 7))
// 		ft_export(ms);
// 	else if (!ft_strncmp("unset", ms->arguments_tmp[0], 6))
// 		ft_unset(ms);
// 	else if (!ft_strncmp("cd", ms->arguments_tmp[0], 3))
// 		ft_cd(ms);
// }

int is_builtin(t_minishell *command) // Takes a command pointer
{
    if (!command || !command->arguments || !command->arguments[0])
        return (0);
    if (!ft_strncmp("exit", command->arguments[0], 5)) // Use arguments[0]
        return (2);
    else if (!ft_strncmp("echo", command->arguments[0], 5))
        return (1);
    else if (!ft_strncmp("env", command->arguments[0], 4))
        return (1);
    else if (!ft_strncmp("pwd", command->arguments[0], 4))
        return (1);
    else if (!ft_strncmp("export", command->arguments[0], 7))
        return (1);
    else if (!ft_strncmp("unset", command->arguments[0], 6))
        return (1);
    else if (!ft_strncmp("cd", command->arguments[0], 3))
        return (1);
    return (0);
}

void	execute_commands_loop(t_minishell *ms, t_minishell *commands, int heredoc_num)
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
		if (is_builtin(current))
			exec_builtin(ms, current);
		else
			execute_external_command(ms, current);
		dup2(ms->ter_in, STDIN_FILENO);
		dup2(ms->ter_out, STDOUT_FILENO);
		current = current->next;
	}
}

void execute_command(t_minishell *ms, t_minishell *token_list, int heredoc_num) {
    t_minishell *commands;
		int saved_stdin;
		int saved_stdout;

    if (!token_list) {
        return;
    }

    commands = parse_tokens_into_commands(token_list, ms); // Pass token_list and ms
    if (!commands) {
        return;
    }

		saved_stdin = dup(STDIN_FILENO);
		saved_stdout = dup(STDOUT_FILENO);
		if (saved_stdin == -1 || saved_stdout == -1)
		{
			perror("dup failed");
			return ;
		}
    ms->heredoc_num = heredoc_num; //keep this

    if (contains_pipe(commands)) { // Check for pipes based on linked list
        handle_pipes(ms, commands);   // Pass the head of the command list
    } else {
        execute_commands_loop(ms, commands, heredoc_num); // Pass head of the command list
    }
    free_command_list(commands);  // Free the command list
		dup2(saved_stdin, STDIN_FILENO);
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdin);
		close(saved_stdout);

}

// void	execute_command(t_minishell *ms, t_minishell *token_list,
// 		int heredoc_num)
// {
// 	t_minishell	*commands;

// 	if (!token_list)
// 	{
// 		ft_printf("Error: No tokens to execute.\n");
// 		return ;
// 	}
// 	commands = parse_tokens_into_commands(token_list);
// 	if (!commands)
// 	{
// 		ft_printf("Error: Parsing tokens into commands.\n");
// 		return ;
// 	}
// 	ms->heredoc_num = heredoc_num;
// 	if (contains_pipe(commands))
// 	{
// 		// printf("DEBUG: Entering piping execute\n");
// 		execute_piped_commands(commands, ms);
// 	}
// 	else
// 	{
// 		// printf("DEBUG: Entering regular execute\n");
// 		execute_commands_loop(ms, commands, heredoc_num);
// 	}
// 	free_command(commands);
// }
