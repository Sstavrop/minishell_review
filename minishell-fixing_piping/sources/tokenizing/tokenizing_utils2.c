/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizing_utils2.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 10:48:50 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/06 13:33:20 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	validate_and_get_quote(const char *input, int *i, t_token_types *type)
{
	char	quote;

	quote = input[*i];
	(*i)++;
	while (input[*i] && input[*i] != quote)
	{
		if (quote == '"' && input[*i] == '\\' && (input[*i + 1] == '"' || input[*i + 1] == '\\' || input[*i + 1] == '$'))
			(*i)++;
		(*i)++;
	}
	if (!input[*i])
	{
		ft_printf("Error: Mismatched '%c' quote at position '%d'\n", quote, *i);
		return (-1);
	}
	if (quote == '\'')
		*type = T_SQUOTE_CONTENT;
	else
		*type = T_DQUOTE_CONTENT;
	return (1);
}

int	create_add_quote(const char *substring, t_minishell **head,
		t_token_types type)
{
	t_minishell	*new_token;

	new_token = create_token(type, ft_strdup(substring));
	if (!new_token || !new_token->value)
	{
		ft_printf("Error: Memory allocation failure at quote tokenizing.\n");
		if (new_token)
			free_token(new_token);
		return (-1);
	}
	add_token(head, new_token);
	return (1);
}



int handle_operator(const char *input, t_minishell **head, int *i) 
{
    t_token_types type;
    int op_length;
    t_minishell *new_token;

    type = set_type(&input[*i]);
    if (type == T_HEREDOC || type == T_APPEND)
        op_length = 2;
    else if (type == T_INPUT || type == T_OUTPUT || type == T_PIPE || type == T_SEMICOLON)
        op_length = 1;
    else
        return (-1);
    new_token = create_token(type, ft_strndup(&input[*i], op_length));
    if (!new_token) 
    {
        fprintf(stderr, "Error: Memory allocation failure for operator token.\n");
        return (-1);
    }
    add_token(head, new_token);
    *i += op_length;
    if (type == T_INPUT || type == T_OUTPUT || type == T_APPEND || type == T_HEREDOC)
    {
        while (ft_iswhitespace(input[*i]))
            (*i)++;
        if(input[*i] == '\0' || ft_isoperator(&input[*i]))//thsi is to cdhecdk if a word exists
        {
            fprintf(stderr, "Error: Redirection operator '%s' requires a filename.\n", new_token->value);
            free_token_list(*head);
            return (-1); //dont free here because we are going to free the list anyways.
        }
    }
    return (1);
}

int handle_word(const char *input, t_minishell **head, int *i) //echo working
{
    int start;
    t_minishell *new_token;

    start = *i;
    if (input[*i] == '-' && (input[*i + 1] == '"' || input[*i + 1] == '\'')) 
	{
        (*i) += 2;
        while (input[*i] && input[*i] != input[start + 1])
            (*i)++;
        if (input[*i] == input[start + 1])
            (*i)++;
    }
	else
	{
        while (input[*i])
		{
            if (input[*i] == '"' || input[*i] == '\'')
			{
                char quote_char = input[*i];
                (*i)++;
                while (input[*i] && input[*i] != quote_char)
                    (*i)++;
                if (input[*i] == quote_char)
                    (*i)++;
            }
            if (!ft_isword(&input[*i]) && input[*i] != '=')
                break;
            (*i)++;
        }
    }
    new_token = create_token(T_WORD, ft_strndup(&input[start], *i - start));
    if (!new_token || !new_token->value)
        return (ft_printf("Error: Memory allocation failure\n"), -1);
    add_token(head, new_token);
    return (1);
}
