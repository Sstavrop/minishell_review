/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prompt.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/11 12:51:20 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:27:43 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*prompt(void)
{
	char	*input;

	input = readline("She11y :> ");
	if (input && *input)
		add_history(input);
	if (!input)
	{
		printf("Until next time!\n");
		rl_clear_history();
		exit(0);
	}
	return (input);
}
