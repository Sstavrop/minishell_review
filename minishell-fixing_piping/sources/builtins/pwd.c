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
        ft_putstr_fd(cwd, STDOUT_FILENO); // Use standard output
        ft_putchar_fd('\n', STDOUT_FILENO);
        ms->last_exit_status = 0; // Success
    }
	else 
	{
        perror("pwd"); // Use perror for system call errors
        ms->last_exit_status = 1; // Failure
    }
}
// void	ft_pwd(void)
// {
// 	char	*cwd;

// 	cwd = getcwd(NULL, 0);
// 	if (cwd == NULL)
// 	{
// 		perror("getcwd failed");
// 		return ;
// 	}
// 	else
// 	{
// 		ft_putstr_fd(cwd, 1);
// 		ft_putchar_fd('\n', 1);
// 		free(cwd);
// 	}
// }
