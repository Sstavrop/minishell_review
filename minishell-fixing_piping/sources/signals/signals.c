/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 14:05:37 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 14:05:38 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int		g_signal_status = 0;

void	signal_handler(int signum)
{
	if (signum == SIGINT && g_signal_status == 1)
	{
		ft_putstr_fd("^C \n", 2);
		g_signal_status = 130;
	}
	else if (signum == SIGINT)
	{
		write(1, "\n", 1);
		rl_replace_line("", 0);
		rl_on_new_line();
		rl_redisplay();
		g_signal_status = 0;
	}
	if (signum == SIGQUIT && g_signal_status == 1)
	{
		ft_putstr_fd("^\\Quit: 3\n", 2);
		g_signal_status = 131;
	}
	else if (signum == SIGQUIT)
	{
		rl_on_new_line();
		rl_redisplay();
		g_signal_status = 0;
	}
}

void	signal_init(void)
{
	signal(SIGQUIT, signal_handler);
	signal(SIGINT, signal_handler);
}
