/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_list.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/15 12:06:59 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/11 09:32:33 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_minishell	*create_token(t_token_types type, char *value)
{
	t_minishell	*new_token;

	new_token = malloc(sizeof(t_minishell));
	if (!new_token)
		return (NULL);
	new_token->type = type;
	new_token->value = value;
	new_token->append = 0;
	new_token->next = NULL;
	return (new_token);
}

void	add_token(t_minishell **head, t_minishell *new_token)
{
	t_minishell	*current;

	if (!*head)
	{
		*head = new_token;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_token;
}

t_minishell	*get_last_token(t_minishell *head)
{
	if (!head)
		return (NULL);
	while (head->next)
		head = head->next;
	return (head);
}

int	skip_whitespaces(const char *input, int i)
{
	while (ft_iswhitespace(input[i]))
		i++;
	return (i);
}
