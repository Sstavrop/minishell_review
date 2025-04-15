/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 10:57:55 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:25:36 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	special_cases(t_minishell *ms, const char *arg_str)
{
	size_t	j;

	j = 0;
	if (!(ft_isalpha(arg_str[0]) || arg_str[0] == '_'))
	{
		fprintf(stderr, "minishell: export: '%s': not a valid identifier\n", arg_str);
		ms->last_exit_status = 1;
		return ;
	}
	j = 1;
    while (arg_str[j]) 
    {
        if (!(ft_isalnum(arg_str[j]) || arg_str[j] == '_')) 
        {
            fprintf(stderr, "minishell: export: `%s': not a valid identifier\n", arg_str);
            ms->last_exit_status = 1; 
            return;
        }
        j++;
    }
    add_env_var(ms, ft_strdup(arg_str), NULL);
}

// void    normal_case(t_minishell *ms, const char *arg_str, char **kvtmp)
// {
//     if (replace_value(ms, kvtmp[0]))
//     {
//         if (kvtmp[1] == 0)
//         {
//             free(replace_value(ms, kvtmp[0])->value);
//             replace_value(ms, kvtmp[0])->value = 0;
//         }
//         else
//         {
//             free(replace_value(ms, kvtmp[0])->value);
//             replace_value(ms, kvtmp[0])->value = ft_strdup(kvtmp[1]);
//         }
//     }
//     else if (ft_strchr(arg_str, '='))
//     {
//         if (kvtmp[1] == 0)
//             add_env_var(ms, ft_strdup(kvtmp[0]), 0);
//         else
//             add_env_var(ms, ft_strdup(kvtmp[0]), ft_strdup(kvtmp[1]));
//     }
// } 

void normal_case(t_minishell *ms, const char *arg_str, char **kvtmp)
{
    t_env *existing_node;
	char *key_copy;
	char *val_copy;

	existing_node = replace_value(ms, kvtmp[0]);
    if (existing_node)
    {
        free(existing_node->value);
        if (kvtmp[1] == NULL) // Handle case like "VAR=" (no value after '=')
            existing_node->value = NULL;
        else
        {
            existing_node->value = ft_strdup(kvtmp[1]); // Set new value
            if (!existing_node->value) 
			{
                perror("minishell: export: strdup failed");
                ms->last_exit_status = 1;
            }
        }
    }
    else if (ft_strchr(arg_str, '=')) // Only add if it didn't exist **BUT** had an '=' (e.g. "NEWVAR=value")
    {
        key_copy = ft_strdup(kvtmp[0]);
        val_copy = NULL;
        if (!key_copy) 
		{
			fprintf(stderr, "minishell: export: memory allocation failed \n");
			ms->last_exit_status = 1;
			return ;
		}
        if (kvtmp[1] != NULL) 
		{ // Only strdup value if it exists
            val_copy = ft_strdup(kvtmp[1]);
			if (!val_copy) 
			{
				fprintf(stderr, "minishell: export: Memory allocation failed\n");
				free(key_copy);
				ms->last_exit_status = 1;	
				return;
			}
        }
        add_env_var(ms, key_copy, val_copy); // add_env_var takes ownership
    }
    // If var didn't exist AND no '=' was present, it's handled by special_cases
}

void	export_init(t_minishell *ms, t_minishell *command)
{
	duplicate(ms);
	ms->export = NULL;
	sort_export(ms);
	if (!command->arguments[1])
		print_export(ms);
}

void	valid_export(t_minishell *ms, const char *arg_str)
{
	char **kvtmp;

	kvtmp = ft_mini_split(ms, arg_str, '=');
	if (!kvtmp)
		return ;
	if (replace_value(ms, kvtmp[0]) || ft_strchr(arg_str, '='))
		normal_case(ms, arg_str, kvtmp);
	else
		special_cases(ms, arg_str);
	free(kvtmp[0]);
	free(kvtmp[1]);
	free(kvtmp);
}

void	ft_export(t_minishell *ms, t_minishell * command)
{
	int	i;

	export_init(ms, command);
	if (command->arguments && command->arguments[1])
	{
		i = 1;
		while (command->arguments[i])
		{
			if (ft_isalpha(command->arguments[i][0]) || command->arguments[i][0] == '_')
				valid_export(ms, command->arguments[i]);
			else
			{
				ms->last_exit_status = 1;
				fprintf(stderr, "minihell: export: %s: not a valid identifier\n",
					command->arguments[i]);
			}
			i++;
		}
	}
	free_export(ms);
	ms->env_dup2 = 0;
}

void	duplicate(t_minishell *ms)
{
	t_env	*tmp;
	t_env	*new;
	char	*key;

	tmp = ms->env_dup;
	while (tmp)
	{
		key = ft_strdup(tmp->key);
		if (!tmp->value)
			new = env_new(key, 0);
		else
			new = env_new(key, ft_strdup(tmp->value));
		if (!new)
			return ;
		env_add_end(&ms->env_dup2, new);
		tmp = tmp->next;
	}
}

void	free_export(t_minishell *ms)
{
	t_env	*tmp;

	while (ms->export)
	{
		tmp = ms->export->next;
		free(ms->export->key);
		free(ms->export->value);
		free(ms->export);
		ms->export = tmp;
	}
}

void	print_export(t_minishell *ms)
{
	t_env	*tmp;

	tmp = ms->export;
	while (tmp)
	{
		if (!tmp->value)
			printf("declare -x %s\n", tmp->key);
		else
			printf("declare -x %s=\"%s\"\n", tmp->key, tmp->value);
		tmp = tmp->next;
	}
}
