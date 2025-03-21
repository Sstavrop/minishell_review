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

t_minishell *create_token(t_token_types type, char *value) 
{
    t_minishell *new_token;

	new_token = (t_minishell *)malloc(sizeof(t_minishell));
    if (!new_token) {
        perror("malloc failed");
        return NULL;
    }

    new_token->type = type;
    new_token->value = value; // Note: We *are* assigning the value here.
    new_token->next = NULL;
    new_token->operator = NO_OPERATOR;
    new_token->append = 0;
    new_token->infile = NULL;
    new_token->outfile = NULL;
    new_token->input_fd = -1;
    new_token->output_fd = -1;
    return new_token;
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
