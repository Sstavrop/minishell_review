/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_tokens.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:38:47 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/06 13:32:57 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// int set_redirection(t_minishell *command, t_minishell *token) //redundant with stage 4
// {
//     if (!command || !token || !token->next)
//         return (0);
//     if (command->input_fd != -1) 
// 	{
//         close(command->input_fd);
//         command->input_fd = -1;
//     }
//     if (command->output_fd != -1) 
// 	{
//         close(command->output_fd);
//         command->output_fd = -1;
//     }
//     if (token->type == T_INPUT)
//     {
//         free(command->infile);
//         command->infile = ft_strdup(token->next->value);
//         if (!command->infile)
// 		{
// 			perror("strdup error");
//             return (0);
// 		}
//         command->operator = INPUT;
//     }
// 	else if (token->type == T_OUTPUT)
//     {
//         free(command->outfile);
//         command->outfile = ft_strdup(token->next->value);
//         if (!command->outfile)
// 		{
// 			perror("strdup error");
//             return (0);
// 		}
//         command->operator = OUTPUT;
//     }
// 	else if (token->type == T_APPEND)
//     {
// 		free(command->outfile);
//         command->outfile = ft_strdup(token->next->value);
//         if (!command->outfile)
//         {
//             perror("strdup failed");
//             return (0);
//         }
//         command->operator = APPEND;
// 		command->append = 1;
//     }
// 	else if (token->type == T_HEREDOC)
//     {
// 		free(command->infile);
//         command->infile = ft_strdup(token->next->value);
//         if (!command->infile)
//         {
//             perror("strdup failed");
//             return (0);
//         }
//         command->operator = T_HEREDOC;
//     }
// 	else
//         return (0);
//     return (1);
// }

// void	handle_word_token(t_minishell **current_command, t_minishell *tokens,
// 		int *command_count, t_minishell **commands) redundant with stage 4
// {
// 	if (*current_command == NULL)
// 	{
// 		*current_command = create_new_command();
// 		if (*current_command == NULL)
// 		{
// 			free_command(*commands);
// 			return ;
// 		}
// 		(*command_count)++;
// 	}
// 	add_argument_to_command(*current_command, tokens->value);
// 	if ((*current_command)->value == NULL)
// 		(*current_command)->value = ft_strdup(tokens->value);
// }

// void	finalize_current_command(t_minishell **commands, t_minishell **current_command) // redundant with stage 4
// {
// 	if (*current_command != NULL)
// 	{
// 		add_command_to_list(commands, *current_command);
// 		*current_command = NULL;
// 	}
// }



// --- Assumed Helper Function Prototypes ---
// t_minishell *create_new_command(void);
// void add_argument_to_command(t_minishell *command, char *arg_value); // Should strdup arg_value
// void free_command_list(t_minishell *commands);

t_minishell *parse_tokens_into_commands(t_minishell *expanded_tokens, t_minishell *ms)  // --- Stage 4 rewrite: Argument Assembly and Command List Creation --- 
{
    t_minishell *command_list_head = NULL; // Head of the list of commands to return
    t_minishell *current_command = NULL;   // Command currently being built
    t_minishell *current_token = expanded_tokens; // Iterator for input tokens    
    char *current_arg_buffer = NULL; // Buffer to build arguments piece by piece
    char *temp_str = NULL;         // For temporary strjoin results
    bool expecting_filename = false;        // State flag for redirections
    t_token_types redirection_type = NO_OPERATOR; // Type of redirection pending

    // --- Initialization ---
    current_arg_buffer = ft_strdup(""); // Start with an empty buffer
    if (!current_arg_buffer) {
        perror("parse_tokens: initial buffer allocation failed");
        return NULL;
    }

    current_command = create_new_command(); // Create the first command structure
    if (!current_command) {
        perror("parse_tokens: create_new_command failed");
        free(current_arg_buffer);
        return NULL;
    }
    command_list_head = current_command;

    // --- Iterate through the expanded token list ---
    while (current_token != NULL) {
        
        if (current_token->type == T_WORD) 
        {
            if (expecting_filename) {
                // --- Handle Filename After Redirection ---
                char *filename_to_add = current_token->value; // Already expanded, quotes removed

                 // Optional: Check for ambiguous redirection (e.g., > out1 > out2)
                if ((redirection_type == T_INPUT || redirection_type == T_HEREDOC) && current_command->infile) {
                     fprintf(stderr, "minishell: ambiguous %s redirect\n", (redirection_type == T_INPUT ? "input" : "heredoc"));
                     free(current_command->infile); // Allow overwriting previous infile spec
                     current_command->infile = NULL;
                 }
                 if ((redirection_type == T_OUTPUT || redirection_type == T_APPEND) && current_command->outfile) {
                     fprintf(stderr, "minishell: ambiguous %s redirect\n", (redirection_type == T_OUTPUT ? "output" : "append"));
                     free(current_command->outfile); // Allow overwriting previous outfile spec
                     current_command->outfile = NULL;
                 }

                // Assign filename copy
                if (redirection_type == T_INPUT || redirection_type == T_HEREDOC) {
                    current_command->infile = ft_strdup(filename_to_add);
                    if (!current_command->infile) { free(current_arg_buffer); free_command_list(command_list_head); return NULL; }
                } else { // OUTPUT or APPEND
                    current_command->outfile = ft_strdup(filename_to_add);
                    if (!current_command->outfile) { free(current_arg_buffer); free_command_list(command_list_head); return NULL; }
                }
                
                // Set operator type on command (used by handle_redirections later)
                current_command->operator = redirection_type; 
                if (redirection_type == T_APPEND) {
                    current_command->append = 1;
                }
                
                expecting_filename = false; // Reset flag - filename received
                redirection_type = NO_OPERATOR;

            } else {
                // --- Append Word To Argument Buffer ---
                // This handles joining adjacent T_WORD tokens (e.g., from a"b"'c')
                temp_str = ft_strjoin(current_arg_buffer, current_token->value);
                if (!temp_str) { free(current_arg_buffer); free_command_list(command_list_head); return NULL; }
                free(current_arg_buffer); // Free the old buffer content
                current_arg_buffer = temp_str; // Assign the newly joined string
            }
        } 
        else if (current_token->type == T_SPACE) 
        {
            // --- Handle Space (Argument Separator) ---
            // If buffer has content, space marks the end of an argument
            if (current_arg_buffer && ft_strlen(current_arg_buffer) > 0) {
                 // Finalize the argument built in the buffer
                 // add_argument_to_command will strdup the buffer content
                add_argument_to_command(current_command, current_arg_buffer); 
                free(current_arg_buffer); // Free the buffer we just added
                current_arg_buffer = ft_strdup(""); // Start new empty buffer for next arg
                if (!current_arg_buffer) { free_command_list(command_list_head); return NULL; }
            }
            // If buffer was empty, just ignore the space (treat multiple spaces as one)
        } 
        else if (current_token->type == T_PIPE /* || current_token->type == T_SEMICOLON */) 
        {
            // --- Handle Pipe (Command Separator) ---

            // Finalize the last argument for the current command (if any)
             if (current_arg_buffer && ft_strlen(current_arg_buffer) > 0) {
                add_argument_to_command(current_command, current_arg_buffer);
                free(current_arg_buffer);
             } else {
                free(current_arg_buffer); // Free the empty buffer
             }
             current_arg_buffer = ft_strdup(""); // Prepare buffer for next command
             if (!current_arg_buffer) { free_command_list(command_list_head); return NULL; }
            
             // Syntax check: Pipe after redirection without filename?
             if (expecting_filename) { 
                 fprintf(stderr, "minishell: syntax error near unexpected token `|'\n");
                 ms->last_exit_status = 2; // Syntax error code
                 free(current_arg_buffer); free_command_list(command_list_head); return NULL; 
             }
             // Syntax check: Pipe with no preceding command/arguments?
              if (!current_command->arguments && !current_command->infile && !current_command->outfile) {
                 fprintf(stderr, "minishell: syntax error near unexpected token `|'\n");
                 ms->last_exit_status = 2;
                 free(current_arg_buffer); free_command_list(command_list_head); return NULL; 
             }

             // Set operator connecting this command structure to the next
             current_command->operator = T_PIPE; 

             // Create and link the next command structure
             t_minishell *next_cmd = create_new_command();
             if (!next_cmd) { free(current_arg_buffer); free_command_list(command_list_head); return NULL; }
             current_command->next_command = next_cmd;
             current_command = next_cmd; // Move focus to the new command structure

        } 
        else if (current_token->type == T_INPUT || current_token->type == T_OUTPUT ||
                   current_token->type == T_APPEND || current_token->type == T_HEREDOC) 
        {
             // --- Handle Redirection Operator ---

             // Finalize any argument preceding the operator
             if (current_arg_buffer && ft_strlen(current_arg_buffer) > 0) {
                add_argument_to_command(current_command, current_arg_buffer);
                free(current_arg_buffer);
                current_arg_buffer = ft_strdup("");
                 if (!current_arg_buffer) { free_command_list(command_list_head); return NULL; }
             } else {
                 // If buffer is empty, still need to free it before potential error return
                 free(current_arg_buffer);
                 current_arg_buffer = NULL; // Mark as freed
             }


             // Syntax check: Consecutive redirection operators? Or filename missing previously?
             if (expecting_filename) { 
                 fprintf(stderr, "minishell: syntax error near unexpected token `%s'\n", current_token->value);
                 ms->last_exit_status = 2;
                 // free(current_arg_buffer); // Already freed or NULL
                 free_command_list(command_list_head); return NULL; 
             }

             // Set state to expect a filename next
             expecting_filename = true;
             redirection_type = current_token->type; // Remember which redirection we saw
             // Reset buffer in case it was NULLed
             if (!current_arg_buffer) {
                  current_arg_buffer = ft_strdup("");
                 if (!current_arg_buffer) { free_command_list(command_list_head); return NULL; }
             }
        }
        
        // Move to the next token from the expanded list
        current_token = current_token->next;
    } // End while loop over tokens

    // --- After the loop ---

    // Finalize any remaining argument text held in the buffer
    if (current_arg_buffer && ft_strlen(current_arg_buffer) > 0) {
         add_argument_to_command(current_command, current_arg_buffer);
    }
    // Free the last buffer used (whether it had content or not)
    free(current_arg_buffer); 

    // Final syntax check: Did we end waiting for a filename?
    if (expecting_filename) { 
        fprintf(stderr, "minishell: syntax error near unexpected token `newline'\n");
        ms->last_exit_status = 2;
        free_command_list(command_list_head); 
        return NULL; 
    }

    // ms parameter is needed for setting exit status on syntax errors
    (void)ms; // Use this if ms isn't used for anything else *in this specific function*

    // Return the head of the fully parsed command list
    return command_list_head;
}

// t_minishell *parse_tokens_into_commands(t_minishell *tokens, t_minishell *ms) 
// {
//     t_minishell *commands;
//     t_minishell *current_command;

//     commands = NULL;
//     current_command = NULL;
//     if (ms == NULL)
//         return (NULL);//ATTENTION!--this does nothing other than to silence a warning by the compiler, but it must stay because its imperative that *ms is there because we need to pass it to otehr shit)
//     if (!tokens)
//         return (NULL);
//     current_command = create_new_command();
//     if (!current_command)
//         return NULL;
//     commands = current_command;
//     while (tokens != NULL) 
//     {
//         if (tokens->type == T_WORD || tokens->type == T_DQUOTE_CONTENT || tokens->type == T_OPTION)
//             add_argument_to_command(current_command, tokens->value);
//         else if (tokens->type == T_INPUT || tokens->type == T_OUTPUT || tokens->type == T_APPEND || tokens->type == T_HEREDOC) 
//         {
//             if (!set_redirection(current_command, tokens)) 
//             {
//                 free_command_list(commands);
//                 return (NULL);
//             }
//             tokens = tokens->next; //skip
//         }
//         else if (tokens->type == T_PIPE || tokens->type == T_SEMICOLON) 
//         {
//             current_command->pipe_count++;
//             if (tokens->type == T_PIPE)
//                 current_command->operator = PIPE;
//             else
//                 current_command->operator = NO_OPERATOR; //Or T_SEMICOLON?
//             t_minishell *new_command = create_new_command();
//             if (!new_command) 
//             {
//                 free_command_list(commands);
//                 return (NULL);
//             }
//             current_command->next_command = new_command; //link commands
//             current_command = new_command; // Move to the new command.
//         }
//         tokens = tokens->next;
//     }
//     return (commands);
// }


