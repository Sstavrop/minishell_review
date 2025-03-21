/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/09 10:21:04 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:25:45 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void ft_pwd(t_minishell *ms) 
{
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) 
	{
        ft_putstr_fd(cwd, STDOUT_FILENO);
        ft_putchar_fd('\n', STDOUT_FILENO);
        ms->last_exit_status = 0;
    }
	else 
	{
        perror("pwd");
        ms->last_exit_status = 1;
    }
}
