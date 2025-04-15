/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_variables.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 09:06:30 by codespace         #+#    #+#             */
/*   Updated: 2025/02/24 15:47:10 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Helper function to expand $? and $VAR within a string segment.
// Returns a newly allocated string with expansions performed.
// Returns NULL on allocation error.
char *expand_string_segment(const char *segment, t_minishell *ms) 
{
    char *result = ft_strdup(""); // Start with an empty string to build upon
    char *var_value_str = NULL;
    char *var_name = NULL;
    char *itoa_str = NULL;
    char *temp_join = NULL;
    char single_char_str[2] = {0}; // For joining single chars
    int i = 0;
    int var_start;

    if (!result) {
        perror("minishell: expand_string_segment: initial strdup failed");
        return (NULL);
    }

    while (segment[i]) {
        if (segment[i] == '$') {
            i++; // Move past '$'
            if (segment[i] == '?') {
                // Handle $?
                itoa_str = ft_itoa(ms->last_exit_status);
                if (!itoa_str) {
                    perror("minishell: expand_string_segment: ft_itoa failed");
                    free(result); return (NULL);
                }
                temp_join = ft_strjoin(result, itoa_str);
                free(result);
                free(itoa_str);
                result = temp_join;
                i++; // Move past '?'
            } else if (ft_isalnum(segment[i]) || segment[i] == '_') {
                // Handle $VAR
                var_start = i;
                while (ft_isalnum(segment[i]) || segment[i] == '_') {
                    i++;
                }
                var_name = ft_strndup(&segment[var_start], i - var_start);
                if (!var_name) {
                    perror("minishell: expand_string_segment: ft_strndup failed");
                    free(result); return (NULL);
                }
                var_value_str = get_environment_value(var_name, ms->env_dup);
                if (var_value_str) 
                { // Append value if found
                    temp_join = ft_strjoin(result, var_value_str);
                    free(result);
                    result = temp_join;
                } 
                else 
                {
                    // If variable not found, append nothing (empty string - strjoin handles NULL?)
                    // Safety: ensure ft_strjoin handles NULL arg or append "" explicitly
                     temp_join = ft_strjoin(result, ""); // Explicitly join empty string
                     free(result);
                     result = temp_join;
                }
                free(var_name);
                // 'i' is already past the variable name
            } 
            else 
            {
                // Lone '$' or followed by invalid char
                temp_join = ft_strjoin(result, "$");
                free(result);
                result = temp_join;
                // 'i' is already past the '$', do nothing more here, loop continues
            }
        } 
        else 
        {
            // Handle normal character
            single_char_str[0] = segment[i];
            temp_join = ft_strjoin(result, single_char_str);
            free(result);
            result = temp_join;
            i++;
        }
        // Check if strjoin failed in any branch
        if (!result) 
        {
             perror("minishell: expand_string_segment: ft_strjoin failed");
             // Potential memory leak here if any intermediate strings weren't freed according to research, such a simple strjoin loop is prone to leaks on failure.
             return (NULL);
        }
    }
    return (result);
}


// Helper to add a token to the end of a list (avoids repeated traversal)
static void add_token_to_end(t_minishell **head, t_minishell **tail, t_minishell *new_token) 
{
    if (!new_token) return;
    if (*head == NULL) {
        *head = new_token;
        *tail = new_token;
    } else {
        (*tail)->next = new_token;
        *tail = new_token;
    }
}

// Stage 2: Expands variables and status in a raw token list.
// Returns the head of a *new* list of processed tokens.
// Frees the original raw_tokens list.
t_minishell *expand_token_list(t_minishell *raw_tokens, t_minishell *ms) 
{
    t_minishell *current = raw_tokens;
    t_minishell *next_raw = NULL;
    t_minishell *new_head = NULL;
    t_minishell *new_tail = NULL;
    t_minishell *processed_token = NULL;
    char *inner_content = NULL;
    char *expanded_value = NULL;
    char *temp_value = NULL; // For strdup results before creating token

    while (current != NULL) {
        processed_token = NULL;
        inner_content = NULL;
        expanded_value = NULL;
        temp_value = NULL;

        switch (current->type) {
            case T_SQUOTE_CONTENT:
                // Extract content without quotes, NO expansion
                if (ft_strlen(current->value) >= 2) { // Ensure quotes exist
                    inner_content = ft_substr(current->value, 1, ft_strlen(current->value) - 2);
                } else {
                    inner_content = ft_strdup(""); // Empty quotes produce empty string
                }
                if (!inner_content) { perror("expand: ft_substr/strdup failed (SQUOTE)"); goto error_cleanup; }
                
                processed_token = create_token(T_WORD, inner_content); // Result is a T_WORD
                if (!processed_token) { free(inner_content); perror("expand: create_token failed (SQUOTE)"); goto error_cleanup; }
                break;

            case T_DQUOTE_CONTENT:
                // Extract content without quotes
                 if (ft_strlen(current->value) >= 2) {
                    inner_content = ft_substr(current->value, 1, ft_strlen(current->value) - 2);
                } else {
                    inner_content = ft_strdup(""); 
                }
                if (!inner_content) { perror("expand: ft_substr/strdup failed (DQUOTE)"); goto error_cleanup; }

                // Expand the inner content
                expanded_value = expand_string_segment(inner_content, ms);
                free(inner_content); // Free the intermediate substring
                if (!expanded_value) { perror("expand: expand_string_segment failed (DQUOTE)"); goto error_cleanup; }

                processed_token = create_token(T_WORD, expanded_value); // Result is a T_WORD
                if (!processed_token) { free(expanded_value); perror("expand: create_token failed (DQUOTE)"); goto error_cleanup; }
                break;

            case T_WORD:
                // Expand the word directly
                expanded_value = expand_string_segment(current->value, ms);
                 if (!expanded_value) { perror("expand: expand_string_segment failed (WORD)"); goto error_cleanup; }

                processed_token = create_token(T_WORD, expanded_value); // Stays T_WORD
                if (!processed_token) { free(expanded_value); perror("expand: create_token failed (WORD)"); goto error_cleanup; }
                break;

            case T_VAR:
                // Look up and expand
                temp_value = get_environment_value(current->value, ms->env_dup);
                if (temp_value) {
                    expanded_value = ft_strdup(temp_value);
                } else {
                    expanded_value = ft_strdup(""); // Unset var expands to empty string
                }
                 if (!expanded_value) { perror("expand: ft_strdup failed (VAR)"); goto error_cleanup; }

                processed_token = create_token(T_WORD, expanded_value); // Result is T_WORD
                if (!processed_token) { free(expanded_value); perror("expand: create_token failed (VAR)"); goto error_cleanup; }
                break;

            case T_EXIT_STATUS:
                // Convert status to string
                expanded_value = ft_itoa(ms->last_exit_status);
                 if (!expanded_value) { perror("expand: ft_itoa failed (EXIT_STATUS)"); goto error_cleanup; }
                
                processed_token = create_token(T_WORD, expanded_value); // Result is T_WORD
                if (!processed_token) { free(expanded_value); perror("expand: create_token failed (EXIT_STATUS)"); goto error_cleanup; }
                break;

            case T_SPACE:
            case T_PIPE:
            case T_INPUT:
            case T_OUTPUT:
            case T_APPEND:
            case T_HEREDOC:
             case T_SEMICOLON: // Pass operators and spaces through unchanged
                temp_value = ft_strdup(current->value); // Need to duplicate value
                 if (!temp_value) { perror("expand: ft_strdup failed (OP/SPACE)"); goto error_cleanup; }
                
                processed_token = create_token(current->type, temp_value); // Keep original type
                if (!processed_token) { free(temp_value); perror("expand: create_token failed (OP/SPACE)"); goto error_cleanup; }
                break;

            default:
                 // Handle unexpected token types if necessary
                 fprintf(stderr, "minishell: expand: Unexpected token type %d\n", current->type);
                 // Maybe create a copy anyway? Or skip? Let's copy for now.
                 temp_value = ft_strdup(current->value ? current->value : "");
                 if (!temp_value) { perror("expand: ft_strdup failed (UNKNOWN)"); goto error_cleanup; }
                 processed_token = create_token(current->type, temp_value);
                 if (!processed_token) { free(temp_value); perror("expand: create_token failed (UNKNOWN)"); goto error_cleanup; }
                 break;
        }
        
        // Add the processed token to the new list
        add_token_to_end(&new_head, &new_tail, processed_token);

        // Move to the next raw token
        next_raw = current->next;
        // We free the raw token structure, but NOT its value, as the value 
        // pointer might have been reused OR needs separate handling if copied.
        // Let's assume create_token takes ownership or value is copied by strdup/substr.
        // It's safer to free the whole raw list *after* the loop.
        // free(current); // Don't free here, free list below.
        current = next_raw;
    }

    // Free the original raw token list structure (values were copied or handled)
    free_token_list(raw_tokens); // Assuming free_token_list only frees the nodes and values?
                                 // Need to be careful not to double-free values if create_token didn't copy.
                                 // Let's assume free_token_list is safe.

    return (new_head);

error_cleanup:
    // If an error occurred, free any partially built new list and the raw list
    fprintf(stderr,"minishell: Error during token expansion, aborting expansion.\n");
    free_token_list(new_head); 
    free_token_list(raw_tokens); // Free original list too
    return (NULL); // Indicate failure
}

char	*get_environment_value(const char *variable_name, t_env *env_list)
{
	while (env_list)
	{
		if (ft_strcmp(env_list->key, variable_name) == 0)
			return (env_list->value);
		env_list = env_list->next;
	}
	return (NULL);
}

// char	*handle_exit_status(char *expanded, int *i, int last_exit_status)
// {
// 	char	*temp;
// 	char	*exit_status_str;

// 	exit_status_str = ft_itoa(last_exit_status);
// 	if (!exit_status_str)
// 		return (NULL);
// 	temp = ft_strjoin(expanded, exit_status_str);
// 	free(exit_status_str);
// 	free(expanded);
// 	*i += 2;
// 	return (temp);
// }

// char	*handle_env_variable(const char *input, char *expanded, int *i,
// 		t_env *env_list)
// {
// 	char	*variable_name;
// 	char	*variable_value;
// 	char	*temp;
// 	int		start;

// 	start = ++(*i);
// 	while (input[*i] && (ft_isalnum(input[*i]) || input[*i] == '_'))
// 		(*i)++;
// 	variable_name = ft_strndup(&input[start], *i - start);
// 	if (!variable_name)
// 		return (NULL);
// 	variable_value = get_environment_value(variable_name, env_list);
// 	if (variable_value)
// 		temp = ft_strjoin(expanded, variable_value);
// 	else
// 		temp = ft_strjoin(expanded, "");
// 	free(expanded);
// 	free(variable_name);
// 	return (temp);
// }

// char	*build_expanded_string(const char *input, int *i, char *expanded,
// 		t_minishell *ms)
// {
// 	char	temp_char[2];

// 	while (input[*i])
// 	{
// 		if (input[*i] == '$')
// 		{
// 			if (input[*i + 1] == '?')
// 				expanded = handle_exit_status(expanded, i,
// 						ms->last_exit_status);
// 			else if (ft_isalnum(input[*i + 1]) || input[*i + 1] == '_')
// 				expanded = handle_env_variable(input, expanded, i, ms->env_dup);
// 			else
// 				expanded = ft_strjoin(expanded, "$");
// 		}
// 		else
// 		{
// 			temp_char[0] = input[*i];
// 			expanded = ft_strjoin(expanded, temp_char);
// 		}
// 		if (!expanded)
// 			return (NULL);
// 		(*i)++;
// 	}
// 	return (expanded);
// }

// char	*expand_variable(const char *input, t_minishell *ms)
// {
//     int		i;
//     char	*expanded;
//     char	temp_char[2];

//     expanded = ft_strdup("");
//     if (!expanded)
//         return (NULL);
//     i = 0;
//     while (input[i])
//     {
//         if (input[i] == '\'' && (i == 0 || input[i - 1] != '\\'))
//             expanded = handle_single_quote(input, &i, expanded);
//         else if (input[i] == '\"')
//             expanded = handle_double_quote(input, &i, expanded, ms);
//         else if (input[i] == '$')
//         {
//             if (input[i + 1] == '?')
//                 expanded = handle_exit_status(expanded, &i, ms->last_exit_status);
//             else if (ft_isalnum(input[i + 1]) || input[i + 1] == '_')
//                 expanded = handle_env_variable(input, expanded, &i, ms->env_dup);
//             else
//             {
//                 temp_char[0] = input[i];
//                 temp_char[1] = '\0';
//                 expanded = ft_strjoin(expanded, temp_char);
//                 i++;
//             }
//         }
//         else
//         {
//             temp_char[0] = input[i];
//             temp_char[1] = '\0';
//             expanded = ft_strjoin(expanded, temp_char);
//             i++;
//         }
//         if (!expanded)
//             return (NULL);
//     }
//     return (expanded);
// }
