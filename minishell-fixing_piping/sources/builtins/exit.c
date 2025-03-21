/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 14:08:28 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:25:24 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int is_numeric(const char *str) 
{
    if (!str || *str == '\0')
        return (0);
    if (*str == '-' || *str == '+')
        str++;
    while (*str)
	{
        if (*str < '0' || *str > '9')
            return (0);
        str++;
    }
    return 1;
}

void ft_exit(t_minishell *ms, t_minishell *command) 
{
    int exit_status;

    exit_status = 0;
    if (command->arguments[1] == NULL) 
        exit_status = ms->last_exit_status;
    else if (command->arguments[2] != NULL) 
    {
         fprintf(stderr, "exit: too many arguments\n");
         ms->last_exit_status = 1;
         return;
    } 
    else if (!is_numeric(command->arguments[1])) 
    {
         fprintf(stderr, "exit: numeric argument required\n");
         exit_status = 255;
    }
    else
        exit_status = ft_atoi(command->arguments[1]);
    exit(exit_status);
}
