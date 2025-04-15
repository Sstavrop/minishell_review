/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 10:58:03 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:25:53 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int is_valid_identifier(const char *str)
{
	int i;

	if (!str || (!ft_isalpha(str[0]) && str[0] != '_'))
		return (0);
	i = 1;
	while (str[i])
	{
		if(!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

void	delete_np(int number, t_env *current)
{
	if (number == 1)
		current->next->previous = NULL;
	else if (number == 2)
	{
		current->previous->next = current->next;
		current->next->previous = current->previous;
	}
}

void	free_unset(t_env *current, int number, t_minishell *ms)
{
	if (number == 0)
	{
		if (!current->previous)
		{
			free(current->key);
			free(current->value);
			current->value = 0;
			free(current);
			current = 0;
			ms->env_dup = 0;
			return ;
		}
		current->previous->next = NULL;
	}
	if (number == 1 || number == 2)
		delete_np(number, current);
	free(current->key);
	free(current->value);
	current->value = 0;
	free(current);
	if (number == 1)
		ms->env_dup = ms->env_dup->next;
}

void	ft_unset(t_minishell *ms, t_minishell *command)
{
	t_env	*current;
	t_env	*node_to_free;
	int		i;
	int		error_occurred;

	error_occurred = 0;
    if (!command->arguments || !command->arguments[1]) 
	{
        ms->last_exit_status = 0;
        return;
    }
    i = 1;
    while (command->arguments[i]) 
    {
        if (!is_valid_identifier(command->arguments[i])) 
		{
            fprintf(stderr, "minishell: unset: `%s': not a valid identifier\n", 
                    command->arguments[i]);
            error_occurred = 1;
            i++;
            continue;
        }
        current = ms->env_dup;
        node_to_free = NULL;
        while (current) 
		{
            if (ft_strcmp(current->key, command->arguments[i]) == 0) 
			{
                node_to_free = current;
                break;
            }
            current = current->next;
        }
        if (node_to_free) 
		{
            if (!node_to_free->next && !node_to_free->previous) // Only node
                free_unset(node_to_free, 0, ms);
			else if (!node_to_free->next) // Tail node
                free_unset(node_to_free, 0, ms);
			else if (!node_to_free->previous) // Head node
                free_unset(node_to_free, 1, ms);
			else  // Middle node
                free_unset(node_to_free, 2, ms);
        }
        i++;
    }
    ms->last_exit_status = error_occurred; 
}
