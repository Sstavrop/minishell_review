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

// Helper function to remove quotes from a string (in-place).
static char *remove_quotes(char *str) {
    if (!str) {
        return NULL;
    }

    int len = strlen(str);
    int j = 0;

    for (int i = 0; i < len; i++) {
        if (str[i] != '\'' && str[i] != '\"') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0'; // Null-terminate the modified string.
    return str;
}

int handle_heredoc(const char *delimiter, char **heredoc_filename, int heredoc_num) {
    char *filename;
    int fd;
    char *line;

    // 1. Generate a unique filename.
   filename = ft_strdup(".heredoc_tmp_"); // Start with a base name.
    if(!filename)
        return -1; //out of memory

    char *num_str = ft_itoa(heredoc_num); //turn into str
    if(!num_str)
    {
        free(filename); //free allocated mem
        return -1;
    }

    char *temp = ft_strjoin(filename, num_str); //combine name and num
    free(filename); //free old name
    free(num_str);
    if(!temp)
        return -1;
    filename = temp; //filename now has name + num

    *heredoc_filename = filename; // Pass ownership to caller


    // 2. Open the temporary file for writing.
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600); // 0600: Read/write for owner only.
    if (fd == -1) {
        perror("open (heredoc)");
        return -1;
    }

    // 3. Remove quotes from the delimiter.
    char *unquoted_delimiter = remove_quotes(ft_strdup(delimiter));
    if (!unquoted_delimiter) {
        close(fd);
        return -1;
    }
    // 4. Read input lines until the delimiter is encountered.
    while (1) {
        line = readline("> "); // Use readline for consistent input handling.
        if (!line) {
            // Handle EOF (Ctrl+D).
            fprintf(stderr, "minishell: warning: here-document delimited by end-of-file (wanted `%s').\n", unquoted_delimiter);
            break;
        }

		// Trim the newline from the line *before* comparison.
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0'; // Remove the trailing newline
            line_len--; // Adjust length
        }

        if (ft_strncmp(line, unquoted_delimiter, ft_strlen(unquoted_delimiter)) == 0) {
            free(line);
            break; // Stop when the delimiter is found.
        }

        // Write the line to the temp file, include newline
        write(fd, line, ft_strlen(line));
        write(fd, "\n", 1);
        free(line);
    }

    // 5. Clean up and close.
    free(unquoted_delimiter); // Free the unquoted delimiter. *CRITICAL*
    close(fd);

    return 0; // Success.
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

            // *Remove quotes from the delimiter BEFORE calling handle_heredoc*
            char *unquoted_delimiter = ft_strdup(command->infile); // Duplicate first!
            if (!unquoted_delimiter) {
                perror("strdup");
                return -1;
            }
            remove_quotes(unquoted_delimiter); // Remove quotes in place.

            if (handle_heredoc(unquoted_delimiter, &heredoc_filename, heredoc_num) != 0) {
                // Handle the error.  handle_heredoc should print an error message.
                free(unquoted_delimiter); // Free the unquoted delimiter.
                return -1; // Return -1 to indicate failure.
            }

            free(unquoted_delimiter); // Free the unquoted delimiter *after* calling handle_heredoc.

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