/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pather.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 14:52:44 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/17 11:45:17 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char   *handle_absolute_or_relative(char *cmd)
{
    if (ft_strchr(cmd, '/') && access(cmd, X_OK | F_OK) == 0)
        return (ft_strdup(cmd));
    return (NULL);
}

char	*join_path(char *dir, char *cmd)
{
	char	*tmp;
	char	*full_path;

	tmp = ft_strjoin(dir, "/");
	if (!tmp)
		return (NULL);
	full_path = ft_strjoin(tmp, cmd);
	free(tmp);
	return (full_path);
}

char	**get_paths_from_env(t_env *env)
{
	char	*path_value;

	path_value = get_environment_value("PATH", env);
	if (!path_value || !*path_value)
		return (NULL);
	return (ft_split(path_value, ':'));
}

char *find_executable_in_paths(char **paths, char *cmd)
{
    int     i;
    char    *full_path;

    if (!paths)
        return (NULL);
    i = 0;
    while (paths[i])
    {
        full_path = join_path(paths[i], cmd);
        if (!full_path)
            return (NULL);
        if (access(full_path, X_OK | F_OK) == 0)
            return (full_path);
        free(full_path);
        i++;
    }
    return (NULL);
}

char *pathfinder(char *cmd, t_env *env) 
{
    char **paths;
    char *finalpath;

    finalpath = handle_absolute_or_relative(cmd);
    if (finalpath)
        return (finalpath);
    paths = get_paths_from_env(env);
    if (!paths)
        return (NULL);
    finalpath = find_executable_in_paths(paths, cmd);
    free_array(paths);
    return (finalpath);
}
