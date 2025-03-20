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

char    *handle_single_quote(const char *input, int *i, char *expanded) 
{
    char    temp_char[2];

    temp_char[1] = '\0'; // I added trhis here to ensure null termination
    (*i)++;
    while (input[*i] && input[*i] != '\'') 
	{
        temp_char[0] = input[*i];
        expanded = ft_strjoin(expanded, temp_char);
        if (!expanded)
            return (NULL);
        (*i)++;
    }
    if (input[*i] == '\'')
        (*i)++;
    return (expanded);
}

char    *handle_double_quote(const char *input, int *i, char *expanded, t_minishell *ms) 
{
    char    temp_char[2];

    temp_char[1] = '\0'; // Added this here to ensure null termination
    (*i)++;
    while (input[*i] && input[*i] != '\"') 
	{
        if (input[*i] == '$' && (input[*i + 1] == '?' || ft_isalnum(input[*i + 1]) || input[*i + 1] == '_')) 
		{
            if (input[*i + 1] == '?')
                expanded = handle_exit_status(expanded, i, ms->last_exit_status);
            else
                expanded = handle_env_variable(input, expanded, i, ms->env_dup);
        } 
		else 
		{
            temp_char[0] = input[*i];
            expanded = ft_strjoin(expanded, temp_char);
        }
        if (!expanded)
            return (NULL);
        (*i)++;
    }
    if (input[*i] == '\"')
        (*i)++;
    return (expanded);
}
