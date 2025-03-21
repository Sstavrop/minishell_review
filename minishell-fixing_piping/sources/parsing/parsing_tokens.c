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

int set_redirection(t_minishell *command, t_minishell *token) 
{
    if (!command || !token || !token->next)
        return (0);
    if (command->input_fd != -1) 
	{
        close(command->input_fd);
        command->input_fd = -1;
    }
    if (command->output_fd != -1) 
	{
        close(command->output_fd);
        command->output_fd = -1;
    }
    if (token->type == T_INPUT)
    {
        free(command->infile);
        command->infile = ft_strdup(token->next->value);
        if (!command->infile)
		{
			perror("strdup error");
            return (0);
		}
        command->operator = INPUT;
    }
	else if (token->type == T_OUTPUT)
    {
        free(command->outfile);
        command->outfile = ft_strdup(token->next->value);
        if (!command->outfile)
		{
			perror("strdup error");
            return 0;
		}
        command->operator = OUTPUT;
    }
	else if (token->type == T_APPEND)
    {
		free(command->outfile);
        command->outfile = ft_strdup(token->next->value);
        if (!command->outfile)
        {
            perror("strdup failed");
            return 0;
        }
        command->operator = APPEND;
		command->append = 1;
    }
	else if (token->type == T_HEREDOC)
    {
		free(command->infile);
        command->infile = ft_strdup(token->next->value);
        if (!command->infile)
        {
            perror("strdup failed");
            return (0);
        }
        command->operator = T_HEREDOC;
    }
	else
        return (0);
    return (1);
}

void	handle_word_token(t_minishell **current_command, t_minishell *tokens,
		int *command_count, t_minishell **commands)
{
	if (*current_command == NULL)
	{
		*current_command = create_new_command();
		if (*current_command == NULL)
		{
			free_command(*commands);
			return ;
		}
		(*command_count)++;
	}
	add_argument_to_command(*current_command, tokens->value);
	if ((*current_command)->value == NULL)
		(*current_command)->value = ft_strdup(tokens->value);
}

void	finalize_current_command(t_minishell **commands,
		t_minishell **current_command)
{
	if (*current_command != NULL)
	{
		add_command_to_list(commands, *current_command);
		*current_command = NULL;
	}
}

t_minishell *parse_tokens_into_commands(t_minishell *tokens, t_minishell *ms) { //definition
    t_minishell *commands = NULL;
    t_minishell *current_command = NULL;

    if (ms == NULL)
        return (NULL);//this does nothing other than to silence a warning by the compiler
    if (!tokens)
        return NULL;

    current_command = create_new_command();
    if (!current_command)
        return NULL;
    commands = current_command;

    while (tokens != NULL) {
        if (tokens->type == T_WORD || tokens->type == T_DOUBLEQUOTE || tokens->type == T_OPTION) {
            add_argument_to_command(current_command, tokens->value);

        }
        else if (tokens->type == T_INPUT || tokens->type == T_OUTPUT || tokens->type == T_APPEND || tokens->type == T_HEREDOC) {
            if (!set_redirection(current_command, tokens)) {
                free_command_list(commands);
                return NULL;
            }
            tokens = tokens->next; //skip
        }
        else if (tokens->type == T_PIPE || tokens->type == T_SEMICOLON) {
            current_command->pipe_count++;
            if (tokens->type == T_PIPE)
                current_command->operator = PIPE;
            else
                current_command->operator = NO_OPERATOR; //Or T_SEMICOLON

            t_minishell *new_command = create_new_command();
            if (!new_command) {
                free_command_list(commands); //free previous commands
                return NULL;
            }
            current_command->next_command = new_command; //link commands
            current_command = new_command; // Move to the new command.
        }
        tokens = tokens->next;
    }
    return (commands);
}

// t_minishell *parse_tokens_into_commands(t_minishell *tokens) {
//     t_minishell *commands = NULL;
//     t_minishell *current_command = NULL;

//     if (!tokens)
//         return NULL;

//     current_command = create_new_command();
//     if (!current_command)
//         return NULL;
//     commands = current_command;

//     while (tokens != NULL) 
//     {
//         if (tokens->type == T_WORD || tokens->type == T_DOUBLEQUOTE || tokens->type == T_OPTION) 
//         {
//             add_argument_to_command(current_command, tokens->value);
//         }
//         else if (tokens->type == T_INPUT || tokens->type == T_OUTPUT || tokens->type == T_APPEND || tokens->type == T_HEREDOC) 
//         {
//             if (!set_redirection(current_command, tokens)) 
//             {
//                 free_command_list(commands);
//                 return NULL;
//             }
//             tokens = tokens->next;
//         }
//         else if (tokens->type == T_PIPE || tokens->type == T_SEMICOLON) { // Re-enable
//             if (tokens->type == T_PIPE)
//                 current_command->operator = PIPE;
//             else
//                 current_command->operator = NO_OPERATOR; //Or T_SEMICOLON

//             t_minishell *new_command = create_new_command();
//             if (!new_command) {
//                 free_command_list(commands);
//                 return NULL;
//             }
//             current_command->next_command = new_command;
//             current_command = new_command;
//         }
//         tokens = tokens->next;
//     }
//     return (commands);
// }
