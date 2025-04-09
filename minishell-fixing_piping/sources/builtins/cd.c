/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 10:57:47 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:24:46 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_env	*replace_value(t_minishell *ms, char *str)
{
	t_env	*tmp;

	tmp = ms->env_dup;
	while (tmp)
	{
		if (ft_strcmp(str, tmp->key) == 0)
			return (tmp);
		tmp = tmp->next;
	}
	return (NULL);
}

char	*get_value(t_minishell *ms, char *str)
{
	t_env	*tmp;

	tmp = ms->env_dup;
	while (tmp)
	{
		if (ft_strcmp(str, tmp->key) == 0)
			return (tmp->value);
		tmp = tmp->next;
	}
	return (NULL);
}

void	set_dir(t_minishell *ms)
{
	char	*cwd;
	t_env	*oldpwd_node;

	cwd = malloc(PATH_MAX);
	getcwd(cwd, PATH_MAX);
	oldpwd_node = replace_value(ms, "OLDPWD");
	if (get_value(ms, "PWD"))
	{
		free(replace_value(ms, "PWD")->value);
		replace_value(ms, "PWD")->value = ft_strdup(cwd);
	}
	if (get_value(ms, "OLDPWD"))
	{
		if (oldpwd_node)
		{
			free(oldpwd_node->value);
			if (ms->oldpwd != NULL)
				oldpwd_node->value = ft_strdup(ms->oldpwd);
			else	
				oldpwd_node->value = ft_strdup("");
		}
		// free(replace_value(ms, "OLDPWD")->value);
		// replace_value(ms, "OLDPWD")->value = ft_strdup(ms->oldpwd);
	}
	free(ms->oldpwd);
	ms->oldpwd = ft_strdup(cwd);
	free(cwd);
}

void	ft_cd(t_minishell *ms)
{
	char	*directory;
	char	*arg;
	char	*tmp;

	arg = ms->arguments_tmp[1];
	if (!arg || (arg[0] == '~' && !arg[1]) || arg[0] == '/')
	{
		directory = go_home(ms);
		if (! directory)
			return ;
		new_dir(ms, directory, arg);
	}
	else
	{
		tmp = 0;
		directory = 0;
		directory = change_directory(ms, directory, tmp, arg);
		new_dir(ms, directory, arg);
		free(directory);
	}
}

void	new_dir(t_minishell	*ms, char *directory, char *argument)
{
	int	new_dir;

	new_dir = chdir(directory);
	if (new_dir == -1)
	{
		printf("cd: %s: No such file or directory\n", argument);
		ms->err = 1;
		return ;
	}
	set_dir(ms);
	// free(ms->oldpwd);
}

char	*go_home(t_minishell *ms)
{
	char	*directory;

	if (ms->arguments_tmp[1] && ms->arguments_tmp[1][0] == '/')
	{
		directory = ms->arguments_tmp[1];
		return (directory);
	}
	else
	{
		directory = get_value(ms, "HOME");
		if (!directory)
		{
			printf("cd: HOME not set\n");
			ms->err = 1;
			return (NULL);
		}
		return (directory);
	}
}

char	*change_tilde(t_minishell *ms, char *dir, char *tmp, char *arg)
{
	if (arg[0] == '~')
	{
		tmp = ft_substr(arg, 1, ft_strlen(ms->arguments_tmp[1]));
		dir = ft_strjoin(get_value(ms, "HOME"), tmp);
		if (!dir)
			return (NULL);
		free (tmp);
		return (dir);
	}
	return (NULL);
}

char	*change_directory(t_minishell *ms, char *dir, char *tmp, char *arg)
{
	char	*cwd;

	if (arg[0] == '~')
	{
		dir = change_tilde(ms, dir, tmp, arg);
		return (dir);
	}
	else
	{
		cwd = malloc(PATH_MAX);
		if (!cwd)
			return (NULL);
		tmp = ft_strjoin("/", ms->arguments_tmp[1]);
		if (!tmp)
			return (NULL);
		getcwd(cwd, PATH_MAX);
		dir = ft_strjoin(cwd, tmp);
		if (!dir)
			return (NULL);
		free(cwd);
		free (tmp);
		return (dir);
	}
}
