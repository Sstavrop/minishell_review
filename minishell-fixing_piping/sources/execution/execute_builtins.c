/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   execute_builtins.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/27 14:06:34 by bjbogisc          #+#    #+#             */
/*   Updated: 2025/03/06 14:19:45 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void exec_builtin(t_minishell *ms, t_minishell *command) 
{
    if (!command || !command->arguments || !command->arguments[0])
        return;
    if (ft_strncmp(command->arguments[0], "exit", 5) == 0)
        ft_exit(ms, command);
    else if (ft_strncmp(command->arguments[0], "cd", 3) == 0)
        ft_cd(ms, command);
    else if (ft_strncmp(command->arguments[0], "pwd", 4) == 0)
        ft_pwd(ms);
    else if (ft_strncmp(command->arguments[0], "echo", 5) == 0)
       ft_echo(ms, command);
    else if (ft_strncmp(command->arguments[0], "env", 4) == 0)
        ft_env(ms, command);
    else if (ft_strncmp(command->arguments[0], "export", 7) == 0)
        ft_export(ms, command);
    else if (ft_strncmp(command->arguments[0], "unset", 6) == 0)
        ft_unset(ms, command);
}

int is_builtin(t_minishell *command)
{
    if (!command || !command->arguments || !command->arguments[0])
        return (0);
    if (!ft_strncmp("exit", command->arguments[0], 5))
        return (2);
    else if (!ft_strncmp("echo", command->arguments[0], 5))
        return (1);
    else if (!ft_strncmp("env", command->arguments[0], 4))
        return (1);
    else if (!ft_strncmp("pwd", command->arguments[0], 4))
        return (1);
    else if (!ft_strncmp("export", command->arguments[0], 7))
        return (1);
    else if (!ft_strncmp("unset", command->arguments[0], 6))
        return (1);
    else if (!ft_strncmp("cd", command->arguments[0], 3))
        return (1);
    return (0);
}

void    execute_commands_loop(t_minishell *ms, t_minishell *commands, int heredoc_num)
{
    t_minishell *current;
    pid_t       child_pid; // To store PID from external command
    int         status;    // To store waitpid status

    current = commands;
    while (current)
    {
        child_pid = -1; // Reset pid for each command in the loop
        status = 0;     // Reset status

        ms->arguments_tmp = current->arguments; // For potentially old builtins
        
        // --- Handle Redirections ---
        if (handle_redirections(current, heredoc_num) < 0)
        {
            // handle_redirections should set ms->last_exit_status
            // Restore FDs before continuing to next command (if any, e.g. after ';')
            // Error checking for dup2 is important in real code
            dup2(ms->ter_in, STDIN_FILENO);  
            dup2(ms->ter_out, STDOUT_FILENO); 
            current = current->next_command; // Move to next command structure
            continue; // Skip execution for this command
        }

        // --- Execute Command ---
        if (is_builtin(current))
        {
            // Builtins run here in the main process for non-piped commands
            exec_builtin(ms, current); 
            // Builtin functions should set ms->last_exit_status directly
        }
        else
        {
            // === Execute External Command ===
            child_pid = execute_external_command(ms, current); // Now returns pid_t

            // Check if fork/exec setup succeeded (pid > 0)
            if (child_pid > 0) 
            {
                // === WAIT for the external command ===
                waitpid(child_pid, &status, 0); 

                // === Update exit status based on wait status ===
                if (WIFEXITED(status)) {
                    ms->last_exit_status = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    // Handle signals (like Ctrl+C killing the child)
                    ms->last_exit_status = 128 + WTERMSIG(status);
                    // Optional: Print signal messages like bash
                    if (WTERMSIG(status) == SIGINT) fprintf(stderr, "\n"); 
                    if (WTERMSIG(status) == SIGQUIT) fprintf(stderr, "Quit: %d\n", WTERMSIG(status)); 
                    g_signal_status = ms->last_exit_status; // Update global if needed
                }
            }
            // If child_pid < 0, execute_external_command already set exit status
        }

        // --- Restore Original FDs ---
        // Restore after *every* command in the loop (redirection or not)
        // Error checking needed for production code
        dup2(ms->ter_in, STDIN_FILENO); 
        dup2(ms->ter_out, STDOUT_FILENO); 
        
        // --- Move to next command --- 
        // (Relevant if supporting ';' which creates linked commands without pipes)
        current = current->next_command; 
    }
}

// Now, this function receives the already parsed command list. 
//It just needs to check for pipes and dispatch to the correct execution function,
// eitehr (handle_pipes or execute_commands_loop).

// Refactored execute_command - Takes COMMAND list head
void execute_command(t_minishell *ms, t_minishell *command_list_head, int heredoc_num) 
{
    if (!command_list_head) 
    {
        return; 
    }

    // 1. Check for pipes directly on the input command list
    //    (Using contains_pipe or checking command_list_head->next_command could work,
    //     but handle_pipes itself checks for next_command, so we can simplify)   
    // A simple way to check: Does the first command have a pipe operator leading out?
    // Or more robustly: Is there more than one command in the list?
    if (command_list_head->next_command != NULL && command_list_head->operator == T_PIPE) 
        // --- Pipeline Execution ---
        // handle_pipes already takes the command list head
        handle_pipes(ms, command_list_head); 
    else 
        // --- Single Command or Non-Piped Sequence Execution ---
        // execute_commands_loop already takes the command list head
        // It will handle the single command case correctly.
        execute_commands_loop(ms, command_list_head, heredoc_num); 
    // 2. Cleanup: This function should NOT free command_list_head.
    //    The caller (main) is responsible for freeing the command list 
    //    *after* execution is fully complete.
    // 3. TODO: Need to handle/reset heredoc_num correctly, maybe here or in main.
    (void)heredoc_num; // Temporarily mark as unused if not handled yet
}