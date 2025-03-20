/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_heredoc.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 14:15:40 by codespace         #+#    #+#             */
/*   Updated: 2025/02/24 11:56:09 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_child_process(char *heredoc_filename, const char *delimiter)
{
	int	fd;

	signal(SIGINT, SIG_DFL);
	fd = open_heredoc_file(heredoc_filename);
	if (fd == -1)
		exit(1);
	if (read_input_and_process(fd, delimiter) == -1)
	{
		close(fd);
		exit(1);
	}
	close(fd);
	exit(0);
}

int	handle_parent_process(pid_t pid, char **heredoc_filename)
{
	int	status;

	waitpid(pid, &status, 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
	{
		ft_printf("Error: Heredoc child process exited with non-zero status\n");
		free(*heredoc_filename);
		return (-1);
	}
	return (1);
}

int	handle_heredoc(const char *delimiter, char **heredoc_filename,
		int heredoc_num)
{
	pid_t	pid;

	if (generate_heredoc_filename(heredoc_filename, heredoc_num) == -1)
		return (-1);
	pid = fork();
	if (pid == 0)
		handle_child_process(*heredoc_filename, delimiter);
	else if (pid > 0)
		return (handle_parent_process(pid, heredoc_filename));
	else
	{
		perror("fork");
		return (-1);
	}
	return (1);
}
