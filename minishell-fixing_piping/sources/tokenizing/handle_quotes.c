/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_quotes.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: codespace <codespace@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 13:27:52 by codespace         #+#    #+#             */
/*   Updated: 2025/02/25 13:41:20 by codespace        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int handle_quotes(const char *input, t_minishell **head, int *i) 
{
    int start;
    int end_quote_idx;
    int len;
    t_token_types token_type;
    char *value;
    t_minishell *new_token;
    char start_quote_char;

    start = *i;
    start_quote_char = input[start];
    if (validate_and_get_quote(input, i, &token_type) == -1)
        return (-1);
    end_quote_idx = *i;
    len = end_quote_idx - start + 1;
    value = ft_strndup(&input[start], len);
    if (!value) 
    {
        perror("ft_strdup failed in handle_quotes");
        return (-1);
    }
    if (start_quote_char == '\'')
        token_type = T_SQUOTE_CONTENT;
    else
        token_type = T_DQUOTE_CONTENT;
    new_token = create_token(token_type, value);
    if (!new_token) 
    {
        free(value);
        return (-1);
    }
    add_token(head, new_token);
    *i = end_quote_idx + 1;
    return (1);
}



// original: int handle_quotes(const char *input, t_minishell **head, int *i, t_minishell *ms) 
// {
//     int start;
//     t_token_types type;
//     char *substring;
//     char *expanded;
//     char *final_str;

//     final_str = NULL;
//     while (ft_isquote(input[*i])) 
// 	{ 
//         start = *i;
//         if (validate_and_get_quote(input, i, &type) == -1)
//             return (-1);
//         substring = ft_strndup(&input[start], *i - start + 1);
//         if (!substring) 
// 		{
//             ft_printf("Error: Memory allocation failure for quote substring.\n");
//             return (-1);
//         }
//         if (type == T_DQUOTE_CONTENT) 
// 		{
//             expanded = expand_variable(substring, ms);
//             free(substring);
//             if (!expanded) 
// 			{
//                 ft_printf("Error: Variable expansion failed\n");
//                 free(final_str);
//                 return (-1);
//             }
//             if (final_str) 
// 			{
//                 char *temp = ft_strjoin(final_str, expanded);
//                 free(final_str);
//                 final_str = temp;
//             } 
// 			else
//                 final_str = ft_strdup(expanded);
//             free(expanded);
//         } 
// 		else 
// 		{
//             if (final_str) 
// 			{
//                 char *temp = ft_strjoin(final_str, substring);
//                 free(final_str);
//                 final_str = temp;
//             } 
// 			else 
//                 final_str = ft_strdup(substring);
//             free(substring);
//         }
//         (*i)++;
//     }
//     if (create_add_quote(final_str, head, T_WORD) == -1) 
// 	{
//         free(final_str);
//         return (-1);
//     }
//     free(final_str);
//     return (1);
// }
