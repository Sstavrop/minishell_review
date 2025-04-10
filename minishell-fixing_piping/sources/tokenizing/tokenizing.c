/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizing.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 14:23:10 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/06 13:33:34 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_valid_flag(const char *arg)
{
	int		i;
	char	quote_char;

	i = 1;
	if (arg[0] != '-')
		return (0);
	if (arg[i] == '"' || arg[i] == '\'')
	{
		quote_char = arg[i];
		i++;
		while (arg[i] && arg[i] != quote_char)
			i++;
		if (arg[i] == quote_char)
			i++;
	}
	while (arg[i] == 'n')
		i++;
	return (arg[i] == '\0');
}

int	validate_input(const char *input)
{
	int		i;
	char	quote;

	if (input == NULL || *input == '\0')
		return (-1);
	i = 0;
	quote = '\0';
	while (input[i])
	{
		if (ft_isquote(input[i]))
		{
			if (quote == '\0')
				quote = input[i];
			else if (quote == input[i])
				quote = '\0';
		}
		else if (quote == '\0' && (input[i] == '\\'))
		{
			ft_printf("Error: Invalid character '\\' at position %d\n", i);
			return (-1);
		}
		else if (quote == '\0' && (input[i] == '<' || input[i] == '>'
				|| input[i] == '|'))
		{
			if ((input[i] == '<' && input[i + 1] == '<') || (input[i] == '>'
					&& input[i + 1] == '>'))
				i++;
			i++;
			while (ft_iswhitespace(input[i]))
				i++;
			if (input[i] == '\0' || ft_isoperator(&input[i]))
			{
				ft_printf("Error: Redirection or pipe without a file name or command at position %d\n", i);
				return (-1);
			}
		}
		i++;
	}
	return (1);
}

int	parse_token(const char *input, t_minishell **head, int *i, t_minishell *ms)
{
	if (ft_isquote(input[*i]))
		return (handle_quotes(input, head, i, ms));
	if (ft_isoperator(&input[*i]))
		return (handle_operator(input, head, i));
	if (ft_isword(&input[*i]))
		return (handle_word(input, head, i));
	(*i)++;
	return (1);
}

t_minishell *process_tokens(const char *input, t_minishell *ms)
{
    t_minishell *head;
    t_minishell *prev_token;
    t_minishell *last_token;
    t_minishell *space_token;
	char		*space_value;
	int i;
	

	head = NULL;
	prev_token = NULL;
	last_token = NULL;
	i = 0;
    while (input[i] != '\0') 
	{
		if (ft_iswhitespace(input[i]))
		{
			while (ft_iswhitespace(input[i]))
				i++;
			space_value = ft_strdup(" ");
			if (!space_value)
			{
				perror("strdup failed for T_SPACE");
				free_token_list(head);
				return (NULL);
			}
			space_token = create_token(T_SPACE, space_value);
			if (!space_token)
			{
				perror("create_token failed for T_SPACE");
				free(space_value);
				free_token_list(head);
				return (NULL);
			}
			add_token(&head, space_token);
			continue;
		}
        if (!ft_isquote(input[i]) && !ft_isoperator(&input[i]) && !ft_isword(&input[i])) 
		{
            ft_printf("Error: Invalid character %c at position %d\n", input[i], i);
            free_token_list(head);
            return (NULL);
        }
        if (parse_token(input, &head, &i, ms) == -1) 
		{
            ft_printf("Error while parsing token\n");
            free_token_list(head);
            return (NULL);
        }
        if (head && head->type == T_SEMICOLON) 
		{
            ft_printf("Error: Unexpected ';' at the beginning\n");
            free_token_list(head);
            return (NULL);
        }
        if (prev_token && (prev_token->type == T_INPUT || prev_token->type == T_OUTPUT
                || prev_token->type == T_HEREDOC || prev_token->type == T_APPEND)) 
			{
            last_token = get_last_token(head);
            if (!last_token || last_token->type != T_WORD) 
			{
                ft_printf("Error: Redirection operator must be followed by a filename\n");
                free_token_list(head);
                return (NULL);
            }
        }
        if (prev_token && prev_token->type == T_SEMICOLON)
            prev_token = NULL;
        prev_token = get_last_token(head);
    }
    return (head);
}


// original: t_minishell *process_tokens(const char *input, t_minishell *ms)
// {
//     t_minishell *head;
//     t_minishell *prev_token;
//     t_minishell *last_token;
//     int i;

// 	head = NULL;
// 	prev_token = NULL;
// 	last_token = NULL;
// 	i = 0;
//     while (input[i] != '\0') 
// 	{
//         i = skip_whitespaces(input, i);
//         if (input[i] == '\0')
//             break;
//         if (!ft_isquote(input[i]) && !ft_isoperator(&input[i]) && !ft_isword(&input[i])) 
// 		{
//             ft_printf("Error: Invalid character %c at position %d\n", input[i], i);
//             free_token_list(head);
//             return (NULL);
//         }
//         if (parse_token(input, &head, &i, ms) == -1) 
// 		{
//             ft_printf("Error while parsing token\n");
//             free_token_list(head);
//             return (NULL);
//         }
//         if (head && head->type == T_SEMICOLON) 
// 		{
//             ft_printf("Error: Unexpected ';' at the beginning\n");
//             free_token_list(head);
//             return (NULL);
//         }
//         if (prev_token && (prev_token->type == T_INPUT || prev_token->type == T_OUTPUT
//                 || prev_token->type == T_HEREDOC || prev_token->type == T_APPEND)) 
// 			{
//             last_token = get_last_token(head);
//             if (!last_token || last_token->type != T_WORD) 
// 			{
//                 ft_printf("Error: Redirection operator must be followed by a filename\n");
//                 free_token_list(head);
//                 return (NULL);
//             }
//         }
//         if (prev_token && prev_token->type == T_SEMICOLON)
//             prev_token = NULL;
//         prev_token = get_last_token(head);
//     }
//     return (head);
// }

t_minishell *tokenize_input(const char *input, t_minishell *ms)
{
    t_minishell *tokens;

    if (validate_input(input) == -1)
        return (NULL);
    tokens = process_tokens(input, ms);
    return (tokens);
}
