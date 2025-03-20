/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strndup.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sstavrop <sstavrop@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 11:23:07 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/01/15 14:32:01 by sstavrop         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strndup(const char *src, int n)
{
	char	*new;
	int		i;

	i = 0;
	new = malloc(n + 1);
	if (!new)
		return (NULL);
	while (*src && i < n)
	{
		new[i] = src[i];
		i++;
	}
	new[i] = '\0';
	return (new);
}
