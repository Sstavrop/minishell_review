/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute_externals.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 11:10:59 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 10:37:43 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	handle_exece_failure(void)
{
	perror("execve failed");
	exit(EXIT_FAILURE);
}

int	count_tokens(t_minishell *token_list)
{
	int	count;

	count = 0;
	while (token_list)
	{
		count++;
		token_list = token_list->next;
	}
	return (count);
}

char **token_list_to_array(t_minishell *token_list)
{
    int         count;
    t_minishell *tmp;
    char        **argv;

    count = count_tokens(token_list);
    argv = malloc((count + 1) * sizeof(char *));
    if (!argv)
        return (NULL);
    tmp = token_list;
    count = 0;
    while (tmp)
    {
        if (tmp->type == T_WORD || tmp->type == T_DOUBLEQUOTE || tmp->type == T_OPTION) 
        {
            argv[count] = ft_strdup(tmp->value);
            if (!argv[count])
            {
                fprintf(stderr, "Error: Failed to duplicate token value\n");
                free_array(argv);//free on allocation failure.
                return (NULL);
            }
            count++;
        }
        tmp = tmp->next;
    }
    argv[count] = NULL;
    return (argv);
}

char **convert_env_to_array(t_env *env_list) 
{
    int num_vars = 0;
    t_env *current = env_list;
    char **env_array;
    int i = 0;
    // 1. Count the number of environment variables.
    while (current) 
    {
        num_vars++;
        current = current->next;
    }
    // 2. Allocate memory for the array of char pointers (+1 for NULL terminator).
    env_array = (char **)malloc((num_vars + 1) * sizeof(char *));
    if (!env_array) 
    {
        perror("malloc failed");
        return NULL; // Return NULL on allocation failure.
    }
    // 3. Iterate through the linked list and create strings.
    current = env_list;
    while (current) 
    {
        // Calculate the required size for "key=value\0".
        size_t len = strlen(current->key) + strlen(current->value) + 2; // +2 for '=' and '\0'
        env_array[i] = (char *)malloc(len * sizeof(char));
        if (!env_array[i]) {
            perror("malloc failed");
            // Clean up previously allocated strings before returning.
            while (i > 0) 
            {
                i--;
                free(env_array[i]);
            }
            free(env_array);
            return NULL;
        }
        // Construct the "key=value" string.
        snprintf(env_array[i], len, "%s=%s", current->key, current->value);
        i++;
        current = current->next;
    }
    // 4. Null-terminate the array.
    env_array[num_vars] = NULL;
    return env_array;
}

void execute_external_command(t_minishell *ms, t_minishell *command)
{
    pid_t   pid;
    char    *cmd_path;
    char    **env_arr;

    if (!command || !command->arguments || !command->arguments[0] || !command->arguments[0][0]) 
    {
        fprintf(stderr, "Error: empty command\n");
        return ;
    }
    cmd_path = pathfinder(command->arguments[0], ms->env_dup);
    if (!cmd_path)
    {
        fprintf(stderr, "Error: Command not found: %s\n", command->arguments[0]);
        return ;
    }
    pid = fork();
    if (pid == 0)
    {
        // Child process
        env_arr = convert_env_to_array(ms->env_dup);
        if (!env_arr)
        {
            free(cmd_path);
            free_command_data(command); // Correct: Free command data in child on error.
            perror("failed to create array from env");
            exit(EXIT_FAILURE);
        }
        if (execve(cmd_path, command->arguments, env_arr) == -1) {
            free_array(env_arr); //CORRECT: free env_arr if execve fails.
            handle_exece_failure();
        }
    }
    else if (pid < 0)
        perror("fork failed");
    else
        // Parent process
        waitpid(pid, NULL, 0);
    free(cmd_path); //free cmd_path in the parent.
}
