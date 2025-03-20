/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 14:08:55 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 14:08:56 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	sort_utils(t_minishell *ms, t_env *current, t_env *temp, t_env *next)
{
	(void)temp;
	(void)next;
	if (ms->export == NULL || ft_strcmp(current->key, ms->export->key) < 0)
	{
		current->next = ms->export;
		if (ms->export)
			ms->export->previous = current;
		ms->export = current;
	}
}

void	sort_export(t_minishell *ms)
{
	t_env	*current;
	t_env	*temp;
	t_env	*next;

	current = ms->env_dup2;
	while (current)
	{
		next = current->next;
		if (ms->export == NULL || ft_strcmp(current->key, ms->export->key) < 0)
			sort_utils(ms, current, temp, next);
		else
		{
			temp = ms->export;
			while (temp->next && ft_strcmp(current->key, temp->next->key) >= 0)
				temp = temp->next;
			current->next = temp->next;
			if (temp->next)
				temp->next->previous = current;
			temp->next = current;
			current->previous = temp;
		}
		current = next;
	}
}
