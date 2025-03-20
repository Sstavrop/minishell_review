/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 12:00:50 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:23:00 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// int	ft_strcmp(const char *str1, const char *str2)
// {
// 	unsigned int	i;
// 	int				result;

// 	i = 0;
// 	while (str1[i] || str2[i])
// 	{
// 		if (str1[i] != str2[i])
// 		{
// 			result = (unsigned char)str1[i] - (unsigned char)str2[i];
// 			return (result);
// 		}
// 		i++;
// 	}
// 	return (0);
// }

int	ft_strcmp(const char *str1, const char *str2)
{
	unsigned int	i;

	i = 0;
	while (str1[i] || str2[i])
	{
		if (str1[i] != str2[i])
			return ((unsigned char)str1[i] - (unsigned char)str2[i]);
		i++;
	}
	return (0);
}
