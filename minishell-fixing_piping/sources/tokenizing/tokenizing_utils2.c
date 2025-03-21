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
		if (quote == '"' && input[*i] == '\\' && (input[*i + 1] == '"'
				|| input[*i + 1] == '\\' || input[*i + 1] == '$'))
			(*i)++;
		(*i)++;
	}
	if (!input[*i])
	{
		ft_printf("Error: Mismatched '%c' quote at position '%d'\n", quote, *i);
		return (-1);
	}
	if (quote == '\'')
		*type = T_SINGLEQUOTE;
	else
		*type = T_DOUBLEQUOTE;
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

int handle_quotes(const char *input, t_minishell **head, int *i, t_minishell *ms) 
{
    int start;
    t_token_types type;
    char *substring;
    char *expanded;
    char *final_str;

    final_str = NULL;
    while (ft_isquote(input[*i])) 
	{ 
        start = *i;
        if (validate_and_get_quote(input, i, &type) == -1)
            return (-1);
        substring = ft_strndup(&input[start], *i - start + 1);
        if (!substring) 
		{
            ft_printf("Error: Memory allocation failure for quote substring.\n");
            return (-1);
        }
        if (type == T_DOUBLEQUOTE) 
		{
            expanded = expand_variable(substring, ms);
            free(substring);
            if (!expanded) 
			{
                ft_printf("Error: Variable expansion failed\n");
                free(final_str);
                return (-1);
            }
            if (final_str) 
			{
                char *temp = ft_strjoin(final_str, expanded);
                free(final_str);
                final_str = temp;
            } 
			else
                final_str = ft_strdup(expanded);
            free(expanded);
        } 
		else 
		{
            if (final_str) 
			{
                char *temp = ft_strjoin(final_str, substring);
                free(final_str);
                final_str = temp;
            } 
			else 
                final_str = ft_strdup(substring);
            free(substring);
        }
        (*i)++;
    }
    if (create_add_quote(final_str, head, T_WORD) == -1) 
	{
        free(final_str);
        return (-1);
    }
    free(final_str);
    return (1);
}

#include "minishell.h"

int handle_operator(const char *input, t_minishell **head, int *i) {
    t_token_types type = set_type(&input[*i]);
    int op_length;

    if (type == T_HEREDOC || type == T_APPEND) {
        op_length = 2;
    } else if (type == T_INPUT || type == T_OUTPUT || type == T_PIPE || type == T_SEMICOLON) {
        op_length = 1;
    } else {
        return -1; // Invalid operator.
    }

    t_minishell *new_token = create_token(type, ft_strndup(&input[*i], op_length)); // Correct
    if (!new_token) {
        fprintf(stderr, "Error: Memory allocation failure for operator token.\n");
        return -1;
    }
    add_token(head, new_token); //we now add token to the list
    *i += op_length;

    // Check for required filename after redirection operators.
    if (type == T_INPUT || type == T_OUTPUT || type == T_APPEND || type == T_HEREDOC)
    {
        while (ft_iswhitespace(input[*i])) { //skip spaces
            (*i)++;
        }
        if(input[*i] == '\0' || ft_isoperator(&input[*i])) //check if word exists
        {
            fprintf(stderr, "Error: Redirection operator '%s' requires a filename.\n", new_token->value);
            free_token_list(*head); // Correct: Free the token list on error.
            return (-1); //we dont free the token here, because we are going to free the list anyways.
        }
    }
    return 1;
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
