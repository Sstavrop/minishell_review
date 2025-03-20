/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 11:51:40 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 12:15:24 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	generate_heredoc_filename(char **heredoc_filename, int heredoc_num)
{
	char	*num;

	ft_printf("Debug: Generating heredoc filename number %d\n", heredoc_num);
	num = ft_itoa(heredoc_num);
	if (!num)
	{
		ft_printf("Error: Failed to convert heredoc number to string\n");
		return (-1);
	}
	*heredoc_filename = ft_strjoin("heredoc_tmp", num);
	free(num);
	if (!(*heredoc_filename))
	{
		ft_printf("Error: Failed to create heredoc filename\n");
		return (-1);
	}
	ft_printf("Debug: Generated heredoc filename: %s\n", *heredoc_filename);
	return (0);
}

int	open_heredoc_file(const char *filename)
{
	int	fd;

	ft_printf("Debug: Attempting to open heredoc file: %s\n", filename);
	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
	{
		ft_printf("Error: Could not create heredoc file: %s\n", filename);
		return (-1);
	}
	ft_printf("Debug: Successfully opened heredoc file with fd: %d\n", fd);
	return (fd);
}

int	write_line_to_heredoc(int fd, char *line)
{
	ft_printf("Debug: Writing line to heredoc: %s\n", line);
	if (write(fd, line, ft_strlen(line)) == -1 || write(fd, "\n", 1) == -1)
	{
		ft_printf("Error: Failed to write line to heredoc file.\n");
		return (-1);
	}
	return (0);
}

int	read_input_and_process(int fd, const char *delimiter)
{
	char	*line;

	ft_printf("Debug: Starting heredoc input with delimiter: %s\n", delimiter);
	while (1)
	{
		line = readline("> ");
		if (!line)
		{
			ft_printf("Error: Unexpected EOF in heredoc\n");
			return (-1);
		}
		ft_printf("Debug: Read line: %s\n", line);
		if (ft_strncmp(line, delimiter, ft_strlen(delimiter)) == 0
			&& ft_strlen(line) == ft_strlen(delimiter))
		{
			ft_printf("Debug: Delimiter found, ending heredoc input\n");
			free(line);
			break ;
		}
		if (write_line_to_heredoc(fd, line) == -1)
		{
			free(line);
			return (-1);
		}
		free(line);
	}
	return (0);
}
