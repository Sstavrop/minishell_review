/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 10:58:11 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:24:58 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include <stdlib.h>

void ft_env(t_minishell *ms, t_minishell *command) 
{
    t_env *tmp;

  if (command->arguments[1]) //check that env has no args
  {
      ms->last_exit_status = 1; //set error status
      fprintf(stderr, "env: too many arguments\n");
      return;
  }
    tmp = ms->env_dup;
    while (tmp) {
        if (tmp->value)
            ft_printf("%s=%s\n", tmp->key, tmp->value);
        tmp = tmp->next;
    }
  ms->last_exit_status = 0; //set last exit status
}

// void	ft_env(t_minishell *ms)
// {
// 	t_env	*tmp;

// 	if (!ms->arguments_tmp[1])
// 	{
// 		tmp = ms->env_dup;
// 		while (tmp)
// 		{
// 			if (tmp->value)
// 				ft_printf("%s=%s\n", tmp->key, tmp->value);
// 			tmp = tmp->next;
// 		}
// 	}
// 	else
// 	{
// 		ms->err = 127;
// 		printf("env: Impossible action\n");
// 	}
// }

void	add_env_var(t_minishell *ms, char *key, char *value)
{
	t_env *new_var;

	new_var = env_new(key, value);
	if (!new_var)
		return ;
	env_add_end(&ms->env_dup, new_var);
}

void env_init(t_minishell *ms) //verison 2
{
    int i = 0;
    char **tmp;

    while (ms->env[i]) {
        tmp = ft_mini_split(ms, ms->env[i], '=');
        if (!tmp) {
            perror("ft_mini_split failed");
            i++;
            continue;
        }
        // Check if splitting was successful *before* using tmp[0] and tmp[1]
        if (!tmp[0] || !strchr(ms->env[i], '=')) {
            fprintf(stderr, "Invalid environment variable: %s\n", ms->env[i]);
            // free_array(tmp); // REMOVE THIS
            i++;
            continue;
        }
        if (!tmp[1])
            tmp[1] = ft_strdup("");  // safe because we check for !tmp above.
        char *key = ft_strdup(tmp[0]);
        char *value = ft_strdup(tmp[1]);
        if (!key || !value) {
            perror("strdup failed");
            // Clean up: free key and value if only one of them was allocated.
            if (key) free(key);
            if (value) free(value);
            // free_array(tmp); // REMOVE THIS
            i++;
            continue;
        }
        // Add the environment variable.
        add_env_var(ms, key, value);
        // Free the temporary split array.  This is *crucial* to avoid memory leaks.
        free_array(tmp); // KEEP THIS. Important, the array is used as a temp
        i++;
    }
}

t_env	*env_new(char *key, char *value)
{
	t_env	*new;

	new = malloc(sizeof(t_env));
	if (!new)
		return (NULL);
	new->key = key;
	new->value = value;
	new->next = 0;
	new->previous = 0;
	return (new);
}

t_env	*env_last(t_env *lst)
{
	if (!lst)
		return (NULL);
	while (lst->next)
		lst = lst->next;
	return (lst);
}

void	env_add_end(t_env **lst, t_env *new)
{
	t_env	*last;

	if (!new)
		return ;
	if (!*lst)
	{
		*lst = new;
		return ;
	}
	last = env_last(*lst);
	last->next = new;
	new->previous = last;
}

int	env_size(t_env *lst)
{
	int	len;

	len = 0;
	while (lst)
	{
		lst = lst->next;
		len++;
	}
	return (len);
}

void	free_env(t_minishell *ms)
{
	t_env	*tmp;

	while (ms->env_dup)
	{
		tmp = ms->env_dup->next;
		free(ms->env_dup->key);
		free(ms->env_dup->value);
		free(ms->env_dup);
		ms->env_dup = tmp;
	}
}
