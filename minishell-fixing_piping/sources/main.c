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
            // Add cases for any other types you have
            default:                 printf("UNKNOWN"); break;
        }
        // Print value safely, enclosed in delimiters for clarity, especially for spaces
        printf(" | Value: [%s]\n", current->value ? current->value : "NULL"); 

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
    if (ms->input_fd != -1) 
    {
        close(ms->input_fd);
        ms->input_fd = -1;
    }
    if (ms->output_fd != -1) 
    {
        close(ms->output_fd);
        ms->output_fd = -1;
    }
    if (ms->arguments_tmp) 
        ms->arguments_tmp = NULL;
    ms->append = 0;
    ms->infile = NULL;
    ms->outfile = NULL;
    ms->operator = NO_OPERATOR;
    ms->pipe_count = 0;
}

int	main(int argc, char **argv, char **envp)
{
	t_minishell	ms;
	t_minishell	*token_list;
	int			heredoc_num;

	(void)argc;
	(void)argv;
	setup_termios();
	init_minishell_first_time(&ms, envp);
	signal_init();
	heredoc_num = 0;
	while (1)
	{
		reset_minishell_state(&ms);
		ms.input = prompt();
        if(!ms.input)
            break;//or exit(EXIT_SUCCESS);
		if (*ms.input == '\0')
		{
			free(ms.input);
			ms.input = NULL;
            continue ;
		}
		token_list = tokenize_input(ms.input);
		if (!token_list)
		{
			free(ms.input);
			continue ;
		}
        print_token_list(token_list);
		execute_command(&ms, token_list, heredoc_num);
		heredoc_num++;
		free(ms.input);
	}
	rl_clear_history();
	free_env(&ms);
	return (0);
}
