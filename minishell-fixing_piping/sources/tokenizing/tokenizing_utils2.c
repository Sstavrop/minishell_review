/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenizing_utils2.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/08 10:48:50 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/06 13:33:20 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	validate_and_get_quote(const char *input, int *i, t_token_types *type)
{
	char	quote;

	quote = input[*i];
	(*i)++;
	while (input[*i] && input[*i] != quote)
	{
		if (quote == '"' && input[*i] == '\\' && (input[*i + 1] == '"' || input[*i + 1] == '\\' || input[*i + 1] == '$'))
			(*i)++;
		(*i)++;
	}
	if (!input[*i])
	{
		ft_printf("Error: Mismatched '%c' quote at position '%d'\n", quote, *i);
		return (-1);
	}
	if (quote == '\'')
		*type = T_SQUOTE_CONTENT;
	else
		*type = T_DQUOTE_CONTENT;
	return (1);
}

int	create_add_quote(const char *substring, t_minishell **head,
		t_token_types type)
{
	t_minishell	*new_token;

	new_token = create_token(type, ft_strdup(substring));
	if (!new_token || !new_token->value)
	{
		ft_printf("Error: Memory allocation failure at quote tokenizing.\n");
		if (new_token)
			free_token(new_token);
		return (-1);
	}
	add_token(head, new_token);
	return (1);
}

int handle_operator(const char *input, t_minishell **head, int *i) 
{
    t_token_types type;
    int op_length;
    t_minishell *new_token;

    type = set_type(&input[*i]);
    if (type == T_HEREDOC || type == T_APPEND)
        op_length = 2;
    else if (type == T_INPUT || type == T_OUTPUT || type == T_PIPE || type == T_SEMICOLON)
        op_length = 1;
    else
        return (-1);
    new_token = create_token(type, ft_strndup(&input[*i], op_length));
    if (!new_token) 
    {
        fprintf(stderr, "Error: Memory allocation failure for operator token.\n");
        return (-1);
    }
    add_token(head, new_token);
    *i += op_length;
    if (type == T_INPUT || type == T_OUTPUT || type == T_APPEND || type == T_HEREDOC)
    {
        while (ft_iswhitespace(input[*i]))
            (*i)++;
        if(input[*i] == '\0' || ft_isoperator(&input[*i]))//thsi is to cdhecdk if a word exists
        {
            fprintf(stderr, "Error: Redirection operator '%s' requires a filename.\n", new_token->value);
            free_token_list(*head);
            return (-1); //dont free here because we are going to free the list anyways.
        }
    }
    return (1);
}

int handle_word(const char *input, t_minishell **head, int *i) //modification 1
{
    int start;
    t_minishell *new_token;
    char *value;
    int var_start;

    start = *i; // Remember the start of the potential T_WORD segment

    while (input[*i]) // Loop through characters
    {
        // Check for boundaries that end the current word segment
        if (ft_iswhitespace(input[*i]) || ft_isoperator(&input[*i]) || ft_isquote(input[*i])) 
            break;
        // Check for variable/status expansion start
        if (input[*i] == '$') 
        {
            // If there was a word part *before* the '$', create its token first
            if (*i > start) 
            {
                value = ft_strndup(&input[start], *i - start);
                if (!value) 
                { 
                    perror("handle_word: ft_strndup failed");
                    return (-1); 
                }
                new_token = create_token(T_WORD, value);
                if (!new_token) 
                {
                    fprintf(stderr, "handle_word: token creation failed\n");
                    free(value);
                    return (-1); 
                }
                add_token(head, new_token);
            }
            // Advance past the '$' itself
            (*i)++; 
            // Check for '$?'
            if (input[*i] == '?') 
            {
                value = ft_strdup("?"); // Value for T_EXIT_STATUS is just "?"
                if (!value) 
                { 
                    perror("handle_word: ft_strndup failed");
                    return (-1); 
                }
                new_token = create_token(T_EXIT_STATUS, value);
                if (!new_token) 
                {
                    fprintf(stderr, "handle_word: token creation failed\n");
                    free(value);
                    return (-1); 
                }
                add_token(head, new_token);
                (*i)++; // Advance past '?'
                start = *i; // Reset start for the next potential token
                continue; // Continue the WHILE loop for characters after $?
            }
            // Check for '$VAR'
            else if (ft_isalnum(input[*i]) || input[*i] == '_') 
            {
                var_start = *i; // Start of the variable name
                printf("[DEBUG handle_word] Starting VAR loop. var_start=%d, current char='%c'\n", var_start, input[*i]);
                while (ft_isalnum(input[*i]) || input[*i] == '_') 
                {
                    printf("[DEBUG handle_word]   VAR loop consuming: index=%d, char='%c'\n", *i, input[*i]);
                    (*i)++; // Consume the variable name
                }
                printf("[DEBUG handle_word] Ended VAR loop. index=%d, char='%c'\n", *i, input[*i]); // See where it stopped
                printf("[DEBUG handle_word]   var_start=%d, calculated_len=%d\n", var_start, *i - var_start); // See length used
                value = ft_strndup(&input[var_start], *i - var_start); // Extract name
                printf("[DEBUG handle_word]   Extracted value: [%s]\n", value ? value : "NULL"); // See what was actually extracted
                if (!value) 
                { 
                    perror("handle_word: ft_strndup failed");
                    return (-1); 
                }                new_token = create_token(T_VAR, value); // Create T_VAR token
                if (!new_token) 
                {
                    fprintf(stderr, "handle_word: token creation failed\n");
                    free(value);
                    return (-1); 
                }
                add_token(head, new_token);
                start = *i; // Reset start for the next potential token
                printf("[DEBUG handle_word]   Added T_VAR token. Reset start index to %d. Continuing loop.\n", start);
                continue; // Continue the WHILE loop for characters after $VAR
            } 
            // Handle '$' followed by something else (or end of string)
            else 
            {
                // The '$' was not followed by '?' or a valid variable start.
                // Treat the '$' as a literal character.
                // We already advanced *i past the '$'. We need to step back
                // so the '$' is included in the next T_WORD token.
                (*i)--; // Step back to the '$'
                // Let the main loop logic handle the '$' as a normal character now.
            }
        }
        // If it wasn't '$' or if '$' was treated as literal, just advance
        (*i)++; 
    } // End of while loop
    // After the loop (boundary found or end of string), create token for the last segment
    if (*i > start) 
    {
        value = ft_strndup(&input[start], *i - start);
        if (!value) 
        { 
            perror("handle_word: ft_strndup failed");
            return (-1); 
        }
        new_token = create_token(T_WORD, value);
        if (!new_token) 
        {
            fprintf(stderr, "handle_word: token creation failed\n");
            free(value);
            return (-1); 
        }
        add_token(head, new_token);
    }
    return (1); // Return success
}
