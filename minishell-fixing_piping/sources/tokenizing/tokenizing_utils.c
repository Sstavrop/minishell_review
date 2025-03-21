/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizing_utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 11:17:54 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:27:12 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	ft_iswhitespace(char c)// i feel like these should just be added to libft
{
	return (c == 32 || (c >= 9 && c <= 13));
}

int	ft_isquote(char c)
{
	return (c == '\'' || c == '"');
}

int	ft_isoperator(const char *str)
{
	if (!str || !str[0])
		return (0);
	if (str[0] == '<' && str[1] == '<')
		return (2);
	if (str[0] == '>' && str[1] == '>')
		return (3);
	if (str[0] == '<')
		return (1);
	if (str[0] == '>')
		return (1);
	if (str[0] == '|')
		return (1);
	if (str[0] == ';')
		return (1);
	return (0);
}

int	ft_isword(const char *str)
{
	return (!ft_iswhitespace(str[0]) && !ft_isquote(str[0])
		&& !ft_isoperator(str));
}

t_token_types	set_type(const char *str)
{
	if (str[0] == '\'')
		return (T_SINGLEQUOTE);
	else if (str[0] == '"')
		return (T_DOUBLEQUOTE);
	else if (str[0] == '<' && str[1] == '<')
		return (T_HEREDOC);
	else if (str[0] == '>' && str[1] == '>')
		return (T_APPEND);
	else if (str[0] == '<')
		return (T_INPUT);
	else if (str[0] == '>')
		return (T_OUTPUT);
	else if (str[0] == '|')
		return (T_PIPE);
	else if (str[0] == ';')
		return (T_SEMICOLON);
	else if (str[0] == '-' && str[1] == 'n')
		return (T_OPTION);
	else
		return (T_WORD);
}
