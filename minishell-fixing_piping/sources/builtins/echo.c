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

void ft_echo(t_minishell *ms) 
{
    int i;
    int newline;
    char *expanded;
    int first_arg; // Flag to track the first argument

    i = 1;
    newline = 1;
    first_arg = 1;
    // Check for valid flags and adjust the newline flag
    while (ms->arguments_tmp[i] && is_valid_flag(ms->arguments_tmp[i])) 
    {
        newline = 0;
        i++;
    }
    // Iterate over arguments and print them
    while (ms->arguments_tmp[i]) 
    {
        expanded = expand_variable(ms->arguments_tmp[i], ms);
        if (!expanded) 
        {
            ft_printf("Error: Memory allocation failure\n");
            return;
        }
        // Only print a space if it's not the first argument
        if (first_arg) 
            first_arg = 0; // First argument processed, change the flag
        else 
            write(1, " ", 1);
        write(1, expanded, ft_strlen(expanded));
        free(expanded);
        i++;
    }
    if (newline)
        write(1, "\n", 1);
}

