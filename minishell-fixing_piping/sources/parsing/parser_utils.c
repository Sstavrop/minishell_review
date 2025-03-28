/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/21 14:19:09 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/24 10:22:50 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	initialize_arguments_array(t_minishell *cmd)
{
	if (cmd->arguments != NULL)
		return ;
	cmd->arguments = malloc(sizeof(char *) * 2);
	if (!cmd->arguments)
	{
		ft_printf("Error: Memory allocation failed for arguments array\n");
		return ;
	}
	cmd->arguments[0] = NULL;
	cmd->arguments[1] = NULL;
}

void	resize_arguments_array(t_minishell *cmd, int new_size)
{
	char	**new_arguments;
	int		i;

	new_arguments = malloc(sizeof(char *) * (new_size + 1));
	if (!new_arguments)
	{
		ft_printf("Error: Memory allocation failed for new arguments.\n");
		return ;
	}
	i = 0;
	while (i < new_size - 1 && cmd->arguments[i])
	{
		new_arguments[i] = cmd->arguments[i];
		i++;
	}
	new_arguments[i] = NULL;
	free(cmd->arguments);
	cmd->arguments = new_arguments;
}

void add_argument_to_command(t_minishell *command, const char *arg)
{
    int num_args;
    char **new_args;

    if (!command || !arg)
        return;
    num_args = 0;
    if (command->arguments) 
    {
        while (command->arguments[num_args] != NULL) 
            num_args++;
    }
    new_args = (char **)realloc(command->arguments, (num_args + 2) * sizeof(char *));
    if (!new_args) 
    {
        perror("realloc failed");
        return;
    }
    command->arguments = new_args;
    command->arguments[num_args] = ft_strdup(arg);
    if (!command->arguments[num_args]) 
    {
        perror("strdup failed");
        return;
    }
    command->arguments[num_args + 1] = NULL;
}

t_minishell *create_new_command(void) 
{
    t_minishell *new_command;
    new_command = (t_minishell *)malloc(sizeof(t_minishell));
    if (!new_command) 
    {
        perror("malloc failed");
        return NULL;
    }
    new_command->arguments = NULL;
    new_command->arguments_tmp = NULL;
    new_command->input_fd = -1;
    new_command->output_fd = -1;
    new_command->infile = NULL;
    new_command->outfile = NULL;
    new_command->append = 0;
    new_command->operator = NO_OPERATOR;
    new_command->next_command = NULL;
    new_command->next = NULL;
    new_command->pipe_count = 0;
    new_command->pid = 0;
    new_command->heredoc_num = 0;
    return new_command;
}

void	add_command_to_list(t_minishell **head, t_minishell *new_command)
{
	t_minishell	*current;

	if (!head || !new_command)
	{
		ft_printf("Error: Invalid commands list or new command\n");
		return ;
	}
	if (!*head)
		*head = new_command;
	else
	{
		current = *head;
		while (current->next)
			current = current->next;
		current->next = new_command;
	}
}
