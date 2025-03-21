/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 10:57:37 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/04 14:07:09 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void ft_echo(t_minishell *ms, t_minishell *command) 
{
    bool newline = true;
    int i;
  
    i = 1;
    if (command->arguments[i] != NULL && ft_strncmp(command->arguments[i], "-n", 3) == 0) 
    {
        newline = false;
        i++;
    }
    while (command->arguments[i] != NULL) 
    {
        ft_putstr_fd(command->arguments[i], STDOUT_FILENO);
        if (command->arguments[i + 1] != NULL)
            ft_putchar_fd(' ', STDOUT_FILENO);
        i++;
    }
    if (newline)
        ft_putchar_fd('\n', STDOUT_FILENO);
    ms->last_exit_status = 0;
}
