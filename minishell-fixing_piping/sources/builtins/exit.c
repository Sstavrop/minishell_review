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

void ft_exit(t_minishell *ms, t_minishell *command) { //now with command
    int exit_status = 0; // Default exit status

    //if no arguments provided
    if (command->arguments[1] == NULL) { // Check for arguments
        exit_status = ms->last_exit_status; // Use previous exit status
    } else if (command->arguments[2] != NULL) {
         //Too many arguments
         fprintf(stderr, "exit: too many arguments\n");
         ms->last_exit_status = 1; //set exit status to an error
         return; //dont exit in this case.
    } else if (!is_numeric(command->arguments[1])) {
         fprintf(stderr, "exit: numeric argument required\n");
         exit_status = 255;
    } else {
        exit_status = ft_atoi(command->arguments[1]); // Use your ft_atoi
    }

    exit(exit_status); // Always exit from ft_exit, with correct status.
}

// void	ft_exit(t_minishell *ms)
// {
// 	int		i;
// 	char	*msg;

// 	msg = "exiting Shell\n";
// 	write(1, msg, ft_strlen(msg));
// 	{
// 		i = -1;
// 		while (ms->arguments[++i])
// 			free(ms->arguments[i]);
// 		free(ms->arguments);
// 		// free(ms->arguments_size);
// 	}
// 	// if we have to handle various exit instances/scnearios
// 	if (!ms->prompt)
// 		exit(ms->err);
// 	else if (ms->arguments_tmp[1] && !ft_isdigit(ms->arguments_tmp[1][0]))
// 	{
// 		printf("minihell: exit: numeric argument required\n");
// 		ms->err = 255;
// 	}
// 	else if (ms->arguments_tmp[1] && !ms->arguments_tmp[2])
// 		exit(ft_atoi(ms->arguments_tmp[1]) % 256);
// 	else if (ms->arguments_tmp[1] && ms->arguments_tmp[2]
// 		&& ft_isdigit(ms->arguments_tmp[1][0]))
// 	{
// 		printf("minihell: exit: too many arguments\n");
// 		ms->err = 1;
// 	}
// 	else
// 		exit(ms->err);
// }

// int	print_err(char *msg)
// {
// 	write(2, msg, ft_strlen(msg));
// 	write(2, "\n", 1);
// 	exit(EXIT_FAILURE);
// }
