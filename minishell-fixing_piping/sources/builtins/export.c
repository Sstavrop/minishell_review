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

void	special_cases(t_minishell *ms, int i)
{
	size_t	j;

	j = -1;
	while (ft_isalpha(ms->arguments_tmp[i][++j]) || ft_isdigit(ms->arguments_tmp[i][j]))
		;
	if (j == ft_strlen(ms->arguments_tmp[i]))
		add_env_var(ms, ft_strdup(ms->arguments_tmp[i]), 0);
	else
	{
		ft_printf("minihell: export: %s not a valid identifier\n",
			ms->arguments_tmp[i]);
		ms->err = 1;
	}
}

void	normal_case(t_minishell *ms, int i, char **kvtmp) /* original */
{
	if (replace_value(ms, kvtmp[0]))
	{
		if (kvtmp[1] == 0)
		{
			free(replace_value(ms, kvtmp[0])->value);
			replace_value(ms, kvtmp[0])->value = 0;
		}
		else
		{
			free(replace_value(ms, kvtmp[0])->value);
			replace_value(ms, kvtmp[0])->value = ft_strdup(kvtmp[1]);
		}
	}
	else if (ft_strchr(ms->arguments_tmp[i], '='))
	{
		if (kvtmp[1] == 0)
			add_env_var(ms, ft_strdup(kvtmp[0]), 0);
		else
			add_env_var(ms, ft_strdup(kvtmp[0]), ft_strdup(kvtmp[1]));
	}
}

void	export_init(t_minishell *ms)
{
	duplicate(ms);
	ms->export = NULL;
	sort_export(ms);
	if (!ms->arguments_tmp[1])
		print_export(ms);
}

void	valid_export(t_minishell *ms, int i) //*original */
{
	char **kvtmp;

	kvtmp = ft_mini_split(ms, ms->arguments_tmp[i], '=');
	if (!kvtmp)
		return ;
	if (replace_value(ms, kvtmp[0]) || ft_strchr(ms->arguments_tmp[i], '='))
		normal_case(ms, i, kvtmp);
	else
		special_cases(ms, i);
	free(kvtmp[0]);
	free(kvtmp[1]);
	free(kvtmp);
}

void	ft_export(t_minishell *ms)
{
	int	i;

	i = 0;
	export_init(ms);
	if (ms->arguments_tmp[1])
	{
		while (ms->arguments_tmp[++i])
		{
			if (ft_isalpha(ms->arguments_tmp[i][0]))
				valid_export(ms, i);
			else
			{
				ms->err = 1;
				printf("minihell: export: %s: not a valid identifier\n",
					ms->arguments_tmp[i]);
			}
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
