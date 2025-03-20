/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_list.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/15 12:19:44 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 14:53:46 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void free_token(t_minishell *token) 
{
    if (!token)
        return;

    if (token->value && (token->type == T_INPUT || token->type == T_OUTPUT || token->type == T_APPEND || token->type == T_HEREDOC)) 
        free(token->value);
    free(token);
}

void	free_token_list(t_minishell *head)
{
	t_minishell	*current;
	t_minishell	*next;

	current = head;
	while (current)
	{
		next = current->next;
		free_token(current);
		current = next;
	}
}

void free_command_data(t_minishell *cmd) 
{
    if (!cmd)
        return;
    if (cmd->arguments) 
    {
        free_array(cmd->arguments);
        cmd->arguments = NULL;
    }
    free(cmd->infile);
    cmd->infile = NULL;
    free(cmd->outfile);
    cmd->outfile = NULL;
}

void free_command_list(t_minishell *commands) 
{
    t_minishell *current;
    t_minishell *next;

    current = commands;
    while (current) 
    {
        next = current->next_command;
        free_command_data(current);
        free(current);
        current = next;
    }
}

void free_command(t_minishell *cmd) 
{
    if (!cmd)
        return;
    free_command_data(cmd);
    free(cmd);
}

void free_array(char **arr) 
{
    int i;

    if (!arr) 
        return;
    i = 0;
    while (arr[i] != NULL) 
    {
        free(arr[i]);
        i++;
    }
    free(arr);
}
