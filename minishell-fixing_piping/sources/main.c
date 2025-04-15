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
// void print_token_list(t_minishell *token_list) 
// {
//     t_minishell *current = token_list;
//     int i = 0;

//     printf("--- Token List Start ---\n");
//     while (current != NULL) 
//     {
//         printf("Token %d: ", i++);
//         switch (current->type) 
//         {
//             case T_WORD:             printf("T_WORD"); break;
//             case T_SQUOTE_CONTENT:   printf("T_SQUOTE_CONTENT"); break;
//             case T_DQUOTE_CONTENT:   printf("T_DQUOTE_CONTENT"); break;
//             case T_VAR:              printf("T_VAR"); break;
//             case T_EXIT_STATUS:      printf("T_EXIT_STATUS"); break;
//             case T_SPACE:            printf("T_SPACE"); break;
//             case T_PIPE:             printf("T_PIPE"); break;
//             case T_INPUT:            printf("T_INPUT"); break;
//             case T_OUTPUT:           printf("T_OUTPUT"); break;
//             case T_HEREDOC:          printf("T_HEREDOC"); break;
//             case T_APPEND:           printf("T_APPEND"); break;
//             case T_SEMICOLON:        printf("T_SEMICOLON"); break;
//             default:                 printf("UNKNOWN"); break;
//         }
//         printf(" | Value: [%s]\n", current->value ? current->value : "NULL"); // this shit is enclosed in delimiters for clarity, especially for spaces
//         current = current->next;
//     }
//     printf("--- Token List End ---\n");
// }

// Helper function to print the final command structures for debugging Stage 4
void print_command_list(t_minishell *command_list)
{
    t_minishell *cmd = command_list;
    int cmd_num = 0;
    printf("\n--- Parsed Command List ---\n");
    while (cmd != NULL) {
        printf("Command %d:\n", cmd_num);
        printf("  Arguments: ");
        if (cmd->arguments) {
            for (int i = 0; cmd->arguments[i]; i++) 
            {
                printf("[%s] ", cmd->arguments[i]);
            }
        } else {
            printf("(NULL)");
        }
        printf("\n");
        printf("  Infile: [%s]\n", cmd->infile ? cmd->infile : "NULL");
        printf("  Outfile: [%s]\n", cmd->outfile ? cmd->outfile : "NULL");
        printf("  Append Flag: %d\n", cmd->append);
        printf("  Operator to Next: ");
        switch (cmd->operator) {
             // Note: Operator now refers to connection to *next* command or redirection type
             // We might need to store redirection type separately if needed here.
             // Let's assume operator stores PIPE or the last redirection type seen for this cmd.
            case T_PIPE: printf("PIPE (|)\n"); break;
            case T_INPUT: printf("INPUT (<) (Filename stored)\n"); break;
            case T_OUTPUT: printf("OUTPUT (>) (Filename stored)\n"); break;
            case T_APPEND: printf("APPEND (>>) (Filename stored)\n"); break;
            case T_HEREDOC: printf("HEREDOC (<<) (Delimiter stored)\n"); break; // Use T_HEREDOC from enum
            default:
                if (cmd->operator == NO_OPERATOR)
                    printf("NO_OPERATOR (End or ;)\n");
                else
                    printf("UNKNOWN/UNHANDLED OPERATOR (%d)\n", cmd->operator);
                break;
        }
        printf("  Next Command Pointer: %p\n", cmd->next_command);
        printf("---------------------------\n");
        cmd = cmd->next_command; // Move to the next command in the pipeline
        cmd_num++;
    }
     printf("--- End Parsed Command List ---\n");
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
    t_minishell *token_list_raw = NULL;
    t_minishell *token_list_expanded = NULL;
    t_minishell *command_list_head = NULL; // To store Stage 4 result
    int         heredoc_num;

    // --- Initial Setup ---
    (void)argc; // Suppress unused variable warning if argc isn't used
    (void)argv; // Suppress unused variable warning if argv isn't used
    
    // Initialize ms struct members FIRST
    init_minishell_first_time(&ms, envp); // Handles env copy etc.
    
    setup_termios(); // Set terminal attributes
    signal_init();   // Set up signal handlers
    heredoc_num = 0; // Initialize counter
    (void)heredoc_num; // Mark as unused for now until execution uses it

    // --- Main Shell Loop ---
    while (1)
    {
        reset_minishell_state(&ms); // Reset per-command state
        ms.input = prompt();        // Get input line (allocates memory)

        // Check for EOF (Ctrl+D)
        if (!ms.input) 
        {
            printf("exit\n");       // Standard behavior on Ctrl+D with empty line
            break;                  // Exit the loop
        }

        // Check for empty input line
        if (*ms.input == '\0') 
        {
            free(ms.input);
            ms.input = NULL; 
            continue;               // Skip processing, get new prompt
        }
        
        // Add valid input to readline history
        add_history(ms.input); 

        // --- Stage 1: Raw Tokenization ---
        token_list_raw = tokenize_input(ms.input); // Gets raw tokens
        if (!token_list_raw) 
        {
            free(ms.input); 
            ms.input = NULL;
            ms.last_exit_status = 2; // Syntax error
            continue;                // Skip to next prompt
        }
        
        // --- Stage 2: Expansion ---
        token_list_expanded = expand_token_list(token_list_raw, &ms); // Expands variables/status
        // Assumption: expand_token_list frees token_list_raw on success or error
        if (!token_list_expanded) 
        {
            // Expansion failed (error message likely printed inside)
            free(ms.input); 
            ms.input = NULL;
            ms.last_exit_status = 1; // Indicate general error
            continue;                // Skip to next prompt
        }

        // --- Stage 4: Joining & Argument Assembly ---
        command_list_head = parse_tokens_into_commands(token_list_expanded, &ms); // Builds command structures
        
        // --- Cleanup Intermediate Token List ---
        // Now that Stage 4 has processed the expanded list, we can free it.
        free_token_list(token_list_expanded);
        token_list_expanded = NULL; 

        // --- Check Stage 4 Result ---
        if (!command_list_head) 
        {
            // Parser failed (syntax error or allocation failure)
            // Parser should have set ms.last_exit_status and maybe printed an error
            free(ms.input); 
            ms.input = NULL;
            continue;                 // Skip to next prompt
        }

        // --- Optional: Debug Print Stage 4 Result ---
        printf("\n--- Parsed Command List ---\n");
        print_command_list(command_list_head);
        printf("--- End Parsed Command List ---\n");

        // --- Stage 5: Execution (Placeholder) ---
        // TODO: Adapt execute_command (or create new exec function, e.g., execute_pipeline) 
        //       to take command_list_head as input instead of a token list.
        printf(">>> Skipping execution for now <<<\n"); 
        // execute_command(&ms, command_list_head, heredoc_num); // Future call

        // --- Cleanup for this iteration ---
        free_command_list(command_list_head); // Free the command structures for this line
        command_list_head = NULL;             
        
        free(ms.input); // Free the raw input line from readline
        ms.input = NULL; 

    } // End while loop

    // --- Final Cleanup ---
    printf("Performing final cleanup...\n"); // Optional debug message
    rl_clear_history();
    free_env(&ms);         
    free(ms.oldpwd);       
    // Ensure terminal FDs are valid before closing
    if (ms.ter_in != -1) close(ms.ter_in);      
    if (ms.ter_out != -1) close(ms.ter_out);
    // Add any other necessary cleanup (e.g., freeing PATH info if separate)

    printf("Minishell exit.\n"); // Optional debug message
    return (ms.last_exit_status); // Exit shell with the last command's status
}
