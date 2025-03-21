/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_redirection.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 10:52:09 by codespace         #+#    #+#             */
/*   Updated: 2025/02/24 11:56:57 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	assign_redirection(t_minishell *cmd, t_minishell *filename_token,
		t_token_types type)
{
	char	*new_value;

	new_value = ft_strdup(filename_token->value);
	if (!new_value)
	{
		ft_printf("Error: Failed to duplicate filename for redirection.\n");
		return (-1);
	}
	if (type == T_INPUT || type == T_HEREDOC)
	{
		if (cmd->infile)
			free(cmd->infile);
		cmd->infile = new_value;
		cmd->type = type;
		cmd->input_fd = -1;
	}
	else if (type == T_OUTPUT || type == T_APPEND)
	{
		if (cmd->outfile)
			free(cmd->outfile);
		cmd->outfile = new_value;
		cmd->output_fd = -1;
		cmd->type = type;
	}
	return (0);
}

int	input_redirection(t_minishell *cmd)
{
	if (cmd->infile)
	{
		cmd->input_fd = open(cmd->infile, O_RDONLY);
		if (cmd->input_fd < 0)
		{
			perror("Input redirection Error");
			return (-1);
		}
		dup2(cmd->input_fd, STDIN_FILENO);
		close(cmd->input_fd);
	}
	return (0);
}

int	output_redirection(t_minishell *cmd)
{
	int	flags;

	if (cmd->outfile)
	{
		if (cmd->type == T_APPEND)
		{
			ft_printf("Debug: Append mode\n");
			flags = O_WRONLY | O_CREAT | O_APPEND;
		}
		else
		{
			ft_printf("Debug: Truncate mode\n");
			flags = O_WRONLY | O_CREAT | O_TRUNC;
		}
		ft_printf("Debug: Opening output file: %s\n", cmd->outfile);
		cmd->output_fd = open(cmd->outfile, flags, 0644);
		if (cmd->output_fd < 0)
		{
			perror("Output redirection Error");
			return (-1);
		}
		dup2(cmd->output_fd, STDOUT_FILENO);
		close(cmd->output_fd);
	}
	return (0);
}

int	heredoc_redirection(t_minishell *cmd, int heredoc_num)
{
	int		result;
	char	*heredoc_filename;

	if (cmd->infile)
	{
		result = handle_heredoc(cmd->infile, &heredoc_filename, heredoc_num);
		if (result < 0)
		{
			ft_printf("Error: Failed to handle heredoc\n");
			return (result);
		}
		if (cmd->infile)
			free(cmd->infile);
		cmd->infile = ft_strdup(heredoc_filename);
		free(heredoc_filename);
		if (!cmd->infile)
		{
			ft_printf("Error: Failed to duplicate heredoc filename\n");
			return (-1);
		}
	}
	return (0);
}

int handle_redirections(t_minishell *command, int heredoc_num) {
    int fd_in = -1;
    int fd_out = -1;

    if (command->infile) {
        if (command->operator == INPUT) {
            fd_in = open(command->infile, O_RDONLY); // Open for reading
            if (fd_in == -1) {
                perror("open");
                return -1;
            }
        } else if (command->operator == T_HEREDOC) {
            // --- HERE'S THE HEREDOC HANDLING ---
            char *heredoc_filename = NULL; // Initialize to NULL.
            if (handle_heredoc(command->infile, &heredoc_filename, heredoc_num) != 0) {
                // Handle the error.  handle_heredoc should print an error message.
                return -1; // Return -1 to indicate failure.
            }

            // If handle_heredoc was successful, heredoc_filename now points
            // to the name of the temporary file.
            fd_in = open(heredoc_filename, O_RDONLY);
            free(heredoc_filename); // Free the filename *after* opening the file.

            if (fd_in == -1) {
                perror("open (heredoc)"); // More specific error message.
                return -1;
            }
        }

        if (fd_in != -1) { // Only dup2 if we actually opened a file.
             if (dup2(fd_in, STDIN_FILENO) == -1) {
                perror("dup2");
                close(fd_in);
                return -1;
            }
            close(fd_in); // Close the original fd after dup2.
        }
    }

  // ... (rest of handle_redirections - output redirection - remains the same) ...
    if (command->outfile) {
        if (command->operator == OUTPUT) {
            fd_out = open(command->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else if (command->operator == APPEND) {
            fd_out = open(command->outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }

        if (fd_out == -1) {
            perror("open");
            return -1;
        }
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(fd_out);
            return -1;
        }
        close(fd_out);
    }

    return 0; // Success

}

// int	handle_redirections(t_minishell *cmd, int heredoc_num)
// {
// 	int	result;

// 	if (cmd->type == T_HEREDOC && cmd->infile)
// 	{
// 		result = heredoc_redirection(cmd, heredoc_num);
// 		if (result < 0)
// 			return (result);
// 	}
// 	result = input_redirection(cmd);
// 	if (result < 0)
// 		return (result);
// 	result = output_redirection(cmd);
// 	if (result < 0)
// 		return (result);
// 	if (cmd->type == T_HEREDOC && cmd->infile)
// 	{
// 		unlink(cmd->infile);
// 		free(cmd->infile);
// 		cmd->infile = NULL;
// 	}
// 	return (result);
// }
