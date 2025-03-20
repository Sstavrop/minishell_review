/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_mini_split.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 14:08:38 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 14:08:39 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	word_counter(const char *s, char c)
{
	int	count;
	int	i;
	int	in_word;

	count = 0;
	i = 0;
	in_word = 0;
	while (s[i])
	{
		if (s[i] != c && !in_word)
		{
			in_word = 1;
			count++;
		}
		else if (s[i] == c)
			in_word = 0;
		i++;
	}
	return (count);
}

char	*str_maker(const char *s, int start, int end)
{
	char	*str;
	int		i;

	str = malloc((end - start + 1) * sizeof(char));
	if (!str)
		return (NULL);
	i = 0;
	while (start < end)
		str[i++] = s[start++];
	str[i] = '\0';
	return (str);
}

char	**ft_mini_split(t_minishell *ms, char const *s, char c)
{
	char	**new_strs;
	int		start;

	ms->x = 0;
	ms->i = 0;
	ms->j = 0;
	start = -42;
	new_strs = malloc((word_counter(s, c) + 1) * sizeof(char *));
	if (!s || !new_strs)
		return (0);
	while ((size_t)ms->i <= ft_strlen(s))
	{
		if (s[ms->i] != c && start < 0)
			start = ms->i;
		else if ((s[ms->i] == c || !s[ms->i]) && start >= 0 && (!ms->x
				|| !s[ms->i]))
		{
			ms->x++;
			new_strs[ms->j++] = str_maker(s, start, ms->i);
			start = -42;
		}
		ms->i++;
	}
	new_strs[ms->j] = 0;
	return (new_strs);
}
