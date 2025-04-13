/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/05 12:20:17 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/02/20 14:45:34 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Function to print token details for debugging Stage 1
void print_token_list(t_minishell *token_list) 
{
    t_minishell *current = token_list;
    int i = 0;

    printf("--- Token List Start ---\n");
    while (current != NULL) 
    {
        printf("Token %d: ", i++);
        switch (current->type) 
        {
            case T_WORD:             printf("T_WORD"); break;
            case T_SQUOTE_CONTENT:   printf("T_SQUOTE_CONTENT"); break;
            case T_DQUOTE_CONTENT:   printf("T_DQUOTE_CONTENT"); break;
            case T_VAR:              printf("T_VAR"); break;
            case T_EXIT_STATUS:      printf("T_EXIT_STATUS"); break;
            case T_SPACE:            printf("T_SPACE"); break;
            case T_PIPE:             printf("T_PIPE"); break;
            case T_INPUT:            printf("T_INPUT"); break;
            case T_OUTPUT:           printf("T_OUTPUT"); break;
            case T_HEREDOC:          printf("T_HEREDOC"); break;
            case T_APPEND:           printf("T_APPEND"); break;
            case T_SEMICOLON:        printf("T_SEMICOLON"); break;
            default:                 printf("UNKNOWN"); break;
        }
        printf(" | Value: [%s]\n", current->value ? current->value : "NULL"); // this shit is enclosed in delimiters for clarity, especially for spaces
        current = current->next;
    }
    printf("--- Token List End ---\n");
}

void	setup_termios(void)
{
	struct termios	termios;

	if (tcgetattr(STDIN_FILENO, &termios) == -1)
		exit(EXIT_FAILURE);
	termios.c_lflag &= ~(ECHOCTL);
	if ((tcsetattr(STDIN_FILENO, TCSANOW, &termios)) == -1)
		exit(EXIT_FAILURE);
}

void init_minishell_first_time(t_minishell *ms, char **env)
{
    ms->ter_in = dup(STDIN_FILENO);
    ms->ter_out = dup(STDOUT_FILENO);
    if (ms->ter_in == -1 || ms->ter_out == -1)
    {
        perror("dup");
        exit(EXIT_FAILURE);
    }
    ms->env = env; 
    ms->env_dup = NULL;
    ms->err_prev = 0;
    ms->x = 0;
    ms->i = 0;
    ms->j = 0;
    ms->err = 0;
    ms->oldpwd = NULL;
    ms->input = NULL;
    ms->value = NULL;
    ms->next = NULL;
    ms->input_fd = -1;
    ms->output_fd = -1;
    ms->next_command = NULL;
    ms->env_dup2 = NULL;
    ms->export = NULL;
    ms->arguments = NULL;
    ms->arguments_tmp = NULL;
    ms->append = 0;
    ms->infile = NULL;
    ms->outfile = NULL;
    ms->operator = NO_OPERATOR;
    ms->pipe_count = 0;
	ms->pid = 0;
    ms->heredoc_num = 0;
    env_init(ms);
}


void reset_minishell_state(t_minishell *ms)
{
    // NOTE to self: Do not reset ms->env_dup, ms->oldpwd, ms->last_exit_status, ms->ter_in, ms->ter_out, ms->input
    // close potential redirection file descriptors from the previous command
    if (ms->input_fd != -1 && ms->input_fd != STDIN_FILENO) // also it woudlnt be the worst idea if you ahve the space to add chjecking for close
        close(ms->input_fd);
    ms->input_fd = -1; 
    if (ms->output_fd != -1 && ms->output_fd != STDOUT_FILENO) 
        close(ms->output_fd);
    ms->output_fd = -1;
    free(ms->infile);     // free allocated filenames (if they are on the main ms struct)
    ms->infile = NULL;
    free(ms->outfile);
    ms->outfile = NULL;
    // Reset flags and temporary pointers
    ms->append = 0;
    ms->operator = NO_OPERATOR; 
    ms->pipe_count = 0;      
    ms->arguments_tmp = NULL; // Builtins will get args from parsed command struct
    ms->err = 0; // Reset per-command error status
}

int main(int argc, char **argv, char **envp)
{
    t_minishell ms;
    t_minishell *token_list_raw = NULL;      // To store Stage 1 result
    t_minishell *token_list_expanded = NULL; // To store Stage 2 result
    // t_minishell *token_list_final = NULL; // For Stage 3/4 result (Future)
    // t_minishell *commands = NULL;         // For Stage 4 result (Future)
    int         heredoc_num;

    // --- Initial Setup ---
    (void)argc; // Suppress unused variable warning if argc isn't used
    (void)argv; // Suppress unused variable warning if argv isn't used
    init_minishell_first_time(&ms, envp); // Handles env copy etc.
    
    setup_termios(); // Set terminal attributes (e.g., disable ECHOCTL)
    signal_init();   // Set up signal handlers (Ctrl+C, Ctrl+\)
    heredoc_num = 0; // Initialize counter for potential heredocs

    // --- Main Shell Loop ---
    while (1)
    {
        reset_minishell_state(&ms); // Reset per-command state (fds, args etc.)
        ms.input = prompt();        // Get input line (allocates memory)

        // Check for EOF (Ctrl+D)
        if (!ms.input) 
        {
            printf("exit\n"); // Standard behavior on Ctrl+D with empty line
            // Perform clean exit cleanup here before breaking
            // free_env(&ms); // free_env might be better called *after* loop
            // rl_clear_history();
            break; 
        }

        // Check for empty input line
        if (*ms.input == '\0') 
        {
            free(ms.input);
            ms.input = NULL; 
            continue; // Skip processing, get new prompt
        }
        // --- Stage 1: Raw Tokenization ---
        token_list_raw = tokenize_input(ms.input); 
        if (!token_list_raw) {
            free(ms.input); 
            ms.input = NULL;
            // Tokenizer likely printed an error. Set generic error status?
            ms.last_exit_status = 1; // Or 2 for syntax errors? Check convention.
            continue; // Skip to next prompt
        }
        
        // Optional: Debug print Stage 1 result
        // printf("\n--- Raw Token List ---\n");
        // print_token_list(token_list_raw);

        // --- Stage 2: Expansion ---
        token_list_expanded = expand_token_list(token_list_raw, &ms);
        // NOTE: expand_token_list should handle freeing token_list_raw itself

        if (!token_list_expanded) {
            // Expansion failed (error message likely printed inside)
            free(ms.input); 
            ms.input = NULL;
            ms.last_exit_status = 1; // Indicate error
            continue; // Skip to next prompt
        }

        // Debug Print Stage 2 Result (Useful for testing)
        printf("\n--- Expanded Token List ---\n");
        print_token_list(token_list_expanded);
        printf("--- End Expanded Token List ---\n");

        // --- Stage 3: Quote Removal (Placeholder - Future Step) ---
        // token_list_final = remove_quotes(token_list_expanded); 
        // // remove_quotes would free token_list_expanded
        // if (!token_list_final) { /* handle error, free ms.input */ continue; }
        // printf("\n--- Final Token List (Quotes Removed) ---\n");
        // print_token_list(token_list_final);
        
        // --- Stage 4: Argument Assembly (Placeholder - Future Step) ---
        // This might build the command list with char** arguments.
        // Or it might be integrated into execute_command/parsing step.
        // commands = parse_and_assemble(token_list_final, &ms);
        // // assemble would free token_list_final
        // if (!commands) { /* handle error, free ms.input */ continue; }


        // --- Stage 5: Execution ---
        // TODO: Adapt execute_command later! 
        // For now, we pass the result of Stage 2 (expanded tokens).
        // execute_command still contains the old parse_tokens_into_commands, 
        // which expects raw tokens and doesn't handle Stage 3/4 yet. 
        // Execution might fail or be incorrect until execute_command/parsing is updated.
        
        // TEMPORARY CALL - Expects token list (Stage 2 output for now)
        execute_command(&ms, token_list_expanded, heredoc_num); 

        // Increment heredoc count *after* potential use in execute_command
        // Maybe reset per command in reset_minishell_state? Depends on design.
        // heredoc_num++; 

        // --- Cleanup for this iteration ---
        // IMPORTANT: Determine if execute_command (or functions it calls like
        // free_command_list) frees the token list it receives. 
        // If YES -> Remove the line below.
        // If NO  -> Keep the line below.
        free_token_list(token_list_expanded); // Free the list from Stage 2
        
        free(ms.input); // Free the raw input line from readline
        ms.input = NULL; 

    } // End while loop

    // --- Final Cleanup ---
    rl_clear_history();
    free_env(&ms);         // Free the duplicated environment list
    free(ms.oldpwd);       // Free oldpwd if it was ever allocated
    close(ms.ter_in);      // Close saved terminal FDs
    close(ms.ter_out);
    // Add any other global cleanup needed

    return (ms.last_exit_status); // Exit shell with the last command's status
}

// int	main(int argc, char **argv, char **envp) //original --or-- pre-stage 2
// {
// 	t_minishell	ms;
// 	t_minishell	*token_list;
// 	int			heredoc_num;

// 	(void)argc;
// 	(void)argv;
// 	setup_termios();
// 	init_minishell_first_time(&ms, envp);
// 	signal_init();
// 	heredoc_num = 0;
// 	while (1)
// 	{
// 		reset_minishell_state(&ms);
// 		ms.input = prompt();
//         if(!ms.input)
//             break;//or exit(EXIT_SUCCESS);
// 		if (*ms.input == '\0')
// 		{
// 			free(ms.input);
// 			ms.input = NULL;
//             continue ;
// 		}
// 		token_list = tokenize_input(ms.input);
// 		if (!token_list)
// 		{
// 			free(ms.input);
// 			continue ;
// 		}
//         print_token_list(token_list);
// 		execute_command(&ms, token_list, heredoc_num);
// 		heredoc_num++;
// 		free(ms.input);
// 	}
// 	rl_clear_history();
// 	free_env(&ms);
// 	return (0);
// }
