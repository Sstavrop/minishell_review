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
        if (type == T_DQUOTE_CONTENT) 
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
