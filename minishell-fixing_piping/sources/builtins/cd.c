/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/06 10:57:47 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 09:24:46 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_env	*replace_value(t_minishell *ms, char *str)
{
	t_env	*tmp;

	tmp = ms->env_dup;
	while (tmp)
	{
		if (ft_strcmp(str, tmp->key) == 0)
			return (tmp);
		tmp = tmp->next;
	}
	return (NULL);
}

char	*get_value(t_minishell *ms, char *str)
{
	t_env	*tmp;

	tmp = ms->env_dup;
	while (tmp)
	{
		if (ft_strcmp(str, tmp->key) == 0)
			return (tmp->value);
		tmp = tmp->next;
	}
	return (NULL);
}

void ft_cd(t_minishell *ms, t_minishell *command) 
{
    char *arg = NULL;
    char *target_dir = NULL; 
    bool target_needs_free = false; 

    // Get the first argument (the potential directory)
    if (command->arguments && command->arguments[1]) {
        arg = command->arguments[1];
    }

    // *** ADD THIS CHECK ***
    // Check if a second argument (at index 2) exists
    if (command->arguments && command->arguments[1] && command->arguments[2]) 
    {
        fprintf(stderr, "minishell: cd: too many arguments\n");
        ms->last_exit_status = 1; // Indicate error
        return; // Stop processing
    }
    // *** END CHECK ***

    // --- Determine Target Directory ---
    if (!arg || strcmp(arg, "~") == 0) { // Handle "cd" and "cd ~"
        // NOTE: Passing "~" to expand_tilde_path here is slightly redundant
        // as the function checks arg[0]=='~', but it works correctly.
        target_dir = expand_tilde_path(ms, "~"); 
        if (!target_dir) return; // Error already handled in helper
        target_needs_free = true;
    } else if (arg[0] == '~') { // Handle "cd ~/..." or other ~ cases
        target_dir = expand_tilde_path(ms, arg); // Call the refactored function
        if (!target_dir) return; // Error already handled in helper
        target_needs_free = true;
    } 
    // Add "cd -" logic here if needed
    else { // Handle regular paths (absolute/relative)
        target_dir = arg; // Pass directly to chdir
        target_needs_free = false;
    }

    // --- Execute ---
    if (target_dir) {
        // Pass original arg for error reporting context
        new_dir(ms, target_dir, command->arguments[1]); 
    } else if (!arg) {
        // This case should have been handled by expand_tilde_path("~") already
        // but adding a fallback just in case.
         fprintf(stderr, "minishell: cd: HOME not set or other error\n");
         ms->last_exit_status = 1;
    }


    // --- Cleanup ---
    if (target_needs_free) {
        free(target_dir);
    }
}
void    new_dir(t_minishell *ms, char *directory_target, char *original_arg_for_error)
{
    char    path_before_cd[PATH_MAX];
    char    path_after_cd[PATH_MAX];
    int     chdir_result;
    t_env   *pwd_node;
    t_env   *oldpwd_node;

    if (!directory_target) return; 

    // 1. Get CWD before
    if (getcwd(path_before_cd, PATH_MAX) == NULL) { 
        // Replace perror with fprintf
        fprintf(stderr, "minishell: cd: error retrieving current directory\n"); 
        ms->last_exit_status = 1; 
        return; 
    }

    // 2. Change directory
    chdir_result = chdir(directory_target);

    // 3. Handle failure
    if (chdir_result == -1) {
        // Replace fprintf using strerror(errno) with a generic message
        // "No such file or directory" covers the most common case.
        fprintf(stderr, "minishell: cd: %s: No such file or directory\n", 
                original_arg_for_error ? original_arg_for_error : directory_target); 
        ms->last_exit_status = 1;
        return;
    }

    // 4. Handle success - Update PWD, OLDPWD, ms->oldpwd
    
    // 4a. Update internal ms->oldpwd
    free(ms->oldpwd); 
    ms->oldpwd = ft_strdup(path_before_cd);
    if (!ms->oldpwd) { 
        // Replace perror with fprintf for malloc failure
        fprintf(stderr, "minishell: cd: memory allocation error\n");
        ms->last_exit_status = 1; 
        // Continuing here might lead to incorrect OLDPWD later, but cd succeeded.
    }

    // 4b. Update OLDPWD env var
    oldpwd_node = replace_value(ms, "OLDPWD");
    if (oldpwd_node) { 
        free(oldpwd_node->value);
        oldpwd_node->value = ft_strdup(path_before_cd); 
        if (!oldpwd_node->value) {
             // Replace perror with fprintf
             fprintf(stderr, "minishell: cd: memory allocation error\n");
             ms->last_exit_status = 1; 
        }
    } 
    // else { // Optionally create OLDPWD if it doesn't exist }

    // 4c. Get new CWD
    if (getcwd(path_after_cd, PATH_MAX) == NULL) {
        // Replace perror with fprintf
        fprintf(stderr, "minishell: cd: error retrieving new directory path\n");
         ms->last_exit_status = 1; // Indicate an issue occurred
         // Cannot update PWD env var if this fails
    } else {
        // 4d. Update PWD env var
        pwd_node = replace_value(ms, "PWD");
        if (pwd_node) { 
            free(pwd_node->value);
            pwd_node->value = ft_strdup(path_after_cd);
            if (!pwd_node->value) { 
                // Replace perror with fprintf
                 fprintf(stderr, "minishell: cd: memory allocation error\n");
                 ms->last_exit_status = 1; // Mark failure occurred
            }
        }
        // else { // Optionally create PWD if it doesn't exist }

        // Set final status only if getcwd succeeded AND PWD update didn't fail allocation
        // If we continued despite OLDPWD/ms->oldpwd strdup fails, status might already be 1.
        if (ms->last_exit_status == 0) 
             ms->last_exit_status = 0; // Confirm Success if no errors hit so far
    }
    // If getcwd after failed, ms->last_exit_status is already 1.
}

char *get_home_directory(t_minishell *ms, const char *arg)
{
    char *home_dir = NULL;
    char *target_dir = NULL;
    char *path_suffix = NULL;

    home_dir = get_value(ms, "HOME"); // Get HOME value
    if (!home_dir || *home_dir == '\0') 
	{
        fprintf(stderr, "minishell: cd: HOME not set\n");
        ms->last_exit_status = 1;
        return (NULL);
    }
    if (!arg || ft_strcmp(arg, "~") == 0) 
	{ // Case: "cd" or "cd ~"
        target_dir = ft_strdup(home_dir);
        if (!target_dir)
			perror("cd: strdup failed");
    } 
	else if (ft_strncmp(arg, "~/", 2) == 0) 
	{ // Case: "cd ~/..."
        path_suffix = (char *)arg + 1; // Point after the '~'
        target_dir = ft_strjoin(home_dir, path_suffix);
        if (!target_dir) perror("cd: strjoin failed");
    } 
    return (target_dir); //caller must free this result
}

char *expand_tilde_path(t_minishell *ms, const char *arg)
{
    char *home_dir = NULL;
    char *suffix = NULL;
    char *result_path = NULL;
    size_t arg_len;

    if (!arg || arg[0] != '~')
        return (NULL);
    home_dir = get_value(ms, "HOME");
    if (!home_dir || *home_dir == '\0') 
	{
        fprintf(stderr, "minishell: cd: HOME not set\n");
        ms->last_exit_status = 1;
        return (NULL);
    }
    arg_len = ft_strlen(arg);
    if (arg_len == 1) 
	{
        result_path = ft_strdup(home_dir);
        if (!result_path) 
		{
            perror("minishell: cd: strdup failed");
            ms->last_exit_status = 1;
        }
        return (result_path);
    }
    else if (arg[1] == '/') 
	{
        suffix = ft_substr(arg, 1, arg_len - 1); 
        if (!suffix) {
            perror("minishell: cd: ft_substr failed");
            ms->last_exit_status = 1;
            return (NULL);
        }
        result_path = ft_strjoin(home_dir, suffix);
        if (!result_path) {
            perror("minishell: cd: ft_strjoin failed");
            ms->last_exit_status = 1;
        }
        free(suffix);
        return (result_path);
    }
    else 
	{
         fprintf(stderr, "minishell: cd: No such user or directory: %s\n", arg); // Mimic error
         ms->last_exit_status = 1;
         return (NULL);
    }
}
