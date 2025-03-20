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

void	ft_unset(t_minishell *ms)
{
	t_env	*current;
	int		i;

	i = 0;
	while (ms->arguments_tmp[++i])
	{
		current = ms->env_dup;
		while (current)
		{
			if (ft_strcmp(current->key, ms->arguments_tmp[i]) == 0)
			{
				if (!current->next)
					free_unset(current, 0, ms);
				else if (!current->previous)
					free_unset(current, 1, ms);
				else
					free_unset(current, 2, ms);
				break ;
			}
			current = current->next;
		}
	}
}
