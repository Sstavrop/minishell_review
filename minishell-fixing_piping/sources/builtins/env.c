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

  if (command->arguments[1])
  {
      ms->last_exit_status = 1;
      fprintf(stderr, "env: too many arguments\n");
      return;
  }
    tmp = ms->env_dup;
    while (tmp) {
        if (tmp->value)
            ft_printf("%s=%s\n", tmp->key, tmp->value);
        tmp = tmp->next;
    }
  ms->last_exit_status = 0;
}

void	add_env_var(t_minishell *ms, char *key, char *value)
{
	t_env *new_var;

	new_var = env_new(key, value);
	if (!new_var)
		return ;
	env_add_end(&ms->env_dup, new_var);
}

void env_init(t_minishell *ms)
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
        if (!tmp[0] || !strchr(ms->env[i], '=')) 
        {
            fprintf(stderr, "Invalid environment variable: %s\n", ms->env[i]);
            i++;
            continue;
        }
        if (!tmp[1])
            tmp[1] = ft_strdup("");
        char *key = ft_strdup(tmp[0]);
        char *value = ft_strdup(tmp[1]);
        if (!key || !value) 
        {
            perror("strdup failed");
            if (key) 
                free(key);
            if (value)
                free(value);
            i++;
            continue;
        }
        add_env_var(ms, key, value);
        free_array(tmp);
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
