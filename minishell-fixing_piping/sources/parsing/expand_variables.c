/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_variables.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 09:06:30 by codespace         #+#    #+#             */
/*   Updated: 2025/02/24 15:47:10 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*get_environment_value(const char *variable_name, t_env *env_list)
{
	while (env_list)
	{
		if (ft_strcmp(env_list->key, variable_name) == 0)
			return (env_list->value);
		env_list = env_list->next;
	}
	return (NULL);
}

char	*handle_exit_status(char *expanded, int *i, int last_exit_status)
{
	char	*temp;
	char	*exit_status_str;

	exit_status_str = ft_itoa(last_exit_status);
	if (!exit_status_str)
		return (NULL);
	temp = ft_strjoin(expanded, exit_status_str);
	free(exit_status_str);
	free(expanded);
	*i += 2;
	return (temp);
}

char	*handle_env_variable(const char *input, char *expanded, int *i,
		t_env *env_list)
{
	char	*variable_name;
	char	*variable_value;
	char	*temp;
	int		start;

	start = ++(*i);
	while (input[*i] && (ft_isalnum(input[*i]) || input[*i] == '_'))
		(*i)++;
	variable_name = ft_strndup(&input[start], *i - start);
	if (!variable_name)
		return (NULL);
	variable_value = get_environment_value(variable_name, env_list);
	if (variable_value)
		temp = ft_strjoin(expanded, variable_value);
	else
		temp = ft_strjoin(expanded, "");
	free(expanded);
	free(variable_name);
	return (temp);
}

char	*build_expanded_string(const char *input, int *i, char *expanded,
		t_minishell *ms)
{
	char	temp_char[2];

	while (input[*i])
	{
		if (input[*i] == '$')
		{
			if (input[*i + 1] == '?')
				expanded = handle_exit_status(expanded, i,
						ms->last_exit_status);
			else if (ft_isalnum(input[*i + 1]) || input[*i + 1] == '_')
				expanded = handle_env_variable(input, expanded, i, ms->env_dup);
			else
				expanded = ft_strjoin(expanded, "$");
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
	return (expanded);
}

char	*expand_variable(const char *input, t_minishell *ms)
{
    int		i;
    char	*expanded;
    char	temp_char[2];

    expanded = ft_strdup("");
    if (!expanded)
        return (NULL);
    i = 0;
    while (input[i])
    {
        if (input[i] == '\'' && (i == 0 || input[i - 1] != '\\'))
            expanded = handle_single_quote(input, &i, expanded);
        else if (input[i] == '\"')
            expanded = handle_double_quote(input, &i, expanded, ms);
        else if (input[i] == '$')
        {
            if (input[i + 1] == '?')
                expanded = handle_exit_status(expanded, &i, ms->last_exit_status);
            else if (ft_isalnum(input[i + 1]) || input[i + 1] == '_')
                expanded = handle_env_variable(input, expanded, &i, ms->env_dup);
            else
            {
                temp_char[0] = input[i];
                temp_char[1] = '\0';
                expanded = ft_strjoin(expanded, temp_char);
                i++;
            }
        }
        else
        {
            temp_char[0] = input[i];
            temp_char[1] = '\0';
            expanded = ft_strjoin(expanded, temp_char);
            i++;
        }
        if (!expanded)
            return (NULL);
    }
    return (expanded);
}
