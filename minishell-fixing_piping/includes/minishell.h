/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bjbogisc <bjbogisc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 10:37:02 by codespace         #+#    #+#             */
/*   Updated: 2025/03/04 15:24:30 by bjbogisc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "../libft/libft.h"
# include <fcntl.h>
# include <limits.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <signal.h>
# include <stdbool.h>
# include <stdio.h>
# include <string.h>
# include <sys/ioctl.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <termios.h>
# include <unistd.h>
# include <stdlib.h>

# define PATH_MAX 4096
# define NO_OPERATOR 0
# define PIPE 1
# define INPUT 2
# define OUTPUT 3
# define APPEND 4

extern int				g_signal_status;

typedef enum e_token_types
{
    T_WORD = 1,          // Unquoted word segment
    T_SQUOTE_CONTENT,    // Content including single quotes '...'
    T_DQUOTE_CONTENT,    // Content including double quotes "..."
    T_VAR,               // Variable name (e.g., "USER", "HOME")
    T_EXIT_STATUS,       // The "?" for $?
    T_SPACE,             // Whitespace sequence
    T_PIPE,              // |
    T_INPUT,             // <
    T_OUTPUT,            // >
    T_HEREDOC,           // <<
    T_APPEND,            // >>
    T_SEMICOLON,          // ; (If you support it)
    T_OPTION// maybe remove or integrate into T_WORD? (i see it used in this function: token_list_to_array)
} t_token_types;

// typedef enum e_token_types
// {
// 	T_SPACES = 1,
// 	T_WORD,
// 	T_PIPE,
// 	T_INPUT,
// 	T_OUTPUT,
// 	T_HEREDOC,
// 	T_APPEND,
// 	T_SQUOTE_CONTENT,
// 	T_DQUOTE_CONTENT,
// 	T_SEMICOLON,
// 	T_OPTION//possibly remove
// }						t_token_types;

typedef struct s_env
{
	char				*key;
	char				*value;
	struct s_env		*next;
	struct s_env		*previous;
}						t_env;

typedef struct s_minishell
{

	int					x;// possibly remove if you change mini_split
	int					i;// possibly remove if you change mini_split
	int					j;// possibly remove if you change mini_split
	int					err;
	int					err_prev;
	int					ter_in;// original stdin fd
	int					ter_out;//riginal stdout fd
	// int					output;// i cant find
	char				**arguments; //arg array for the *current*command
	char				**arguments_tmp;//temp  arguments --Only used for builtins
	// int					*arguments_size;//possibly remove if not used (i think its not used anywhere at all.)
	char				**env; //original environemtn taken from the mains envp
	char				*prompt;
	char				*oldpwd;
	char				*input;//raw input line from readline
	int					last_exit_status;//exit status of the last command
	t_token_types		type;//type of the current token
	char				*value;// value of the current token
	int					append;//flad for append redirection (>>)
	struct s_minishell	*next;//next token in the token list
	t_token_types		operator;//typoe of operator (PIPE, NO_OPERATOR, etc...)
	char				*infile;//input redirection fiile
	char				*outfile;//output redirection file
	int					input_fd;//fd for the input redirection
	int					output_fd;//fd for the output redirection
	int					pipe_count;//number of pipes in the current command line
	pid_t				pid;//PID of chile process
	// int					*pipe_fds;//i think i can remove now
	int					heredoc_num;//trrack heredoc use
	struct s_minishell	*next_command;// pointer to the next command in a pipeline
	t_env				*env_dup;//duplicated environment (linked list)
	t_env				*env_dup2;//can i remove safely?
	t_env				*export;//also i think i can remove
}						t_minishell;

/****************************************************************/
/*																*/
/*							Tokenizing							*/
/*																*/
/****************************************************************/

/* --- handle_quotes.c --- */
char					*handle_single_quote(const char *input, int *i,
							char *expanded);
char					*handle_double_quote(const char *input, int *i,
							char *expanded, t_minishell *ms);
int						handle_quotes(const char *input, t_minishell **head,
								int *i, t_minishell *ms);

/* --- token_list.c --- */
t_minishell				*create_token(t_token_types type, char *value);
void					add_token(t_minishell **head, t_minishell *new_token);
t_minishell				*get_last_token(t_minishell *head);
int						skip_whitespaces(const char *input, int i);

/* --- tokenize_utils.c --- */
int						ft_iswhitespace(char c);
int						ft_isquote(char c);
int						ft_isoperator(const char *str);
int						ft_isword(const char *str);
t_token_types			set_type(const char *str);

/* --- tokenizing_utils2.c --- */
int						validate_and_get_quote(const char *input, int *i,
							t_token_types *type);
int						create_add_quote(const char *substring,
							t_minishell **head, t_token_types type);
int						handle_operator(const char *input, t_minishell **head,
							int *i);
int						handle_word(const char *input, t_minishell **head,
							int *i);

/* --- tokenizing.c --- */
int						is_valid_flag(const char *arg);
int						validate_input(const char *input);
int						parse_token(const char *input, t_minishell **head,
							int *i, t_minishell *ms);
t_minishell				*process_tokens(const char *input, t_minishell *ms);
t_minishell				*tokenize_input(const char *input, t_minishell *ms);

/****************************************************************/
/*																*/
/*							Parsing								*/
/*																*/
/****************************************************************/

/* --- expand_variables.c --- */
char					*get_environment_value(const char *variable_name,
							t_env *env_list);
char					*handle_exit_status(char *expanded, int *i,
							int last_exit_status);
char					*handle_env_variable(const char *input, char *expanded,
							int *i, t_env *env_list);
char					*build_expanded_string(const char *input, int *i,
							char *expanded, t_minishell *ms);
char					*expand_variable(const char *input, t_minishell *ms);

/* --- handle_heredoc.c --- */
int						handle_child_process(char *heredoc_filename,
							const char *delimiter);
int						handle_parent_process(pid_t pid,
							char **heredoc_filename);
int						handle_heredoc(const char *delimiter, char **heredoc_filename, int heredoc_num);

/* --- handle_redirection.c --- */
int						handle_heredoc(const char *delimiter, char **heredoc_filename, int heredoc_num);
int						handle_redirections(t_minishell *command, int heredoc_num);


/* --- heredoc_utils.c --- */
int						generate_heredoc_filename(char **heredoc_filename,
							int heredoc_num);
int						open_heredoc_file(const char *filename);
int						write_line_to_heredoc(int fd, char *line);
int						read_input_and_process(int fd, const char *delimiter);

/* --- parser_utils.c --- */
void					initialize_arguments_array(t_minishell *cmd);
void					resize_arguments_array(t_minishell *cmd, int new_size);
void					add_argument_to_command(t_minishell *command, const char *arg);
t_minishell				*create_new_command(void);
void					add_command_to_list(t_minishell **head,
							t_minishell *new_command);

/* --- parsing_tokens --- */
int						set_redirection(t_minishell *command, t_minishell *token);
void					handle_word_token(t_minishell **current_command,
							t_minishell *tokens, int *command_count,
							t_minishell **commands);
void					finalize_current_command(t_minishell **commands,
							t_minishell **current_command);
t_minishell				*parse_tokens_into_commands(t_minishell *tokens, t_minishell *ms);
int						calculate_num_pipes(t_minishell *commands);


/****************************************************************/
/*																*/
/*							Builtins							*/
/*																*/
/****************************************************************/

/* --- cd.c --- */
t_env					*replace_value(t_minishell *ms, char *str);
void					new_dir(t_minishell *ms, char *directory,
							char *argument);
char					*get_value(t_minishell *ms, char *str);
char					*go_home(t_minishell *ms);
char					*change_tilde(t_minishell *ms, char *dir, char *tmp,
							char *arg);
char					*change_directory(t_minishell *ms, char *dir, char *tmp,
							char *arg);
void					ft_cd(t_minishell *ms);
// void					set_dir(t_minishell *ms);

/* --- echo.c --- */
void					ft_echo(t_minishell *ms, t_minishell *command);

/* --- env.c --- */
void					ft_env(t_minishell *ms, t_minishell *command);
void					add_env_var(t_minishell *ms, char *key, char *value);
void					env_init(t_minishell *ms);
t_env					*env_new(char *key, char *value);
t_env					*env_last(t_env *lst);
void					env_add_end(t_env **lst, t_env *new);
int						env_size(t_env *lst);
void					free_env(t_minishell *ms);

/* --- exit.c --- */
void					ft_exit(t_minishell *ms, t_minishell *command);
int						print_err(char *msg);

/* --- export.c --- */
void					duplicate(t_minishell *ms);
void					valid_export(t_minishell *ms, int i);
void					export_init(t_minishell *ms);
void					ft_export(t_minishell *ms);
void					normal_case(t_minishell *ms, int i, char **kvtmp);
void					special_cases(t_minishell *ms, int i);
void					free_export(t_minishell *ms);
void					print_export(t_minishell *ms);

/* --- pwd.c --- */
void					ft_pwd(t_minishell *ms);

/* --- unset.c --- */
void					ft_unset(t_minishell *ms);

/****************************************************************/
/*																*/
/*							Execution							*/
/*																*/
/****************************************************************/

/* --- execute_builtins.c --- */
void					exec_builtin(t_minishell *ms, t_minishell *command);
int						is_builtin(t_minishell *command);
void					execute_commands_loop(t_minishell *ms, t_minishell *commands, int heredoc_num);
void					execute_command(t_minishell *ms,
							t_minishell *token_list, int heredoc_num);

/* --- handle_pipes.c --- */
int						create_pipe(int pipe_fds[2]);
void					close_fd(int fd);
int						dup2_and_close(int oldfd, int newfd);
void					handle_pipes(t_minishell *ms, t_minishell *commands);
int						contains_pipe(t_minishell *commands);

/* --- pather.c --- */
char					*handle_absolute_or_relative(char *cmd);
char					*join_path(char *dir, char *cmd);
char					**get_paths_from_env(t_env *env);
char					*find_executable_in_paths(char **paths, char *cmd);
char					*pathfinder(char *cmd, t_env *env);

/* --- execute_externals.c --- */
char					**token_list_to_array(t_minishell *token_list);
void					execute_external_command(t_minishell *ms, t_minishell *command);
char					**convert_env_to_array(t_env *env_list);
void					handle_exece_failure(void);
int						count_tokens(t_minishell *token_list);

/****************************************************************/
/*																*/
/*							Utils								*/
/*																*/
/****************************************************************/

/* --- free_list.c --- */
void					free_token(t_minishell *token);
void					free_token_list(t_minishell *head);
void					free_command_data(t_minishell *command);
void					free_command(t_minishell *command);
void					free_array(char **array);
void					free_command_list(t_minishell *commands);


/* --- prompt.c --- */
char					*prompt(void);

/* --- ft_mini_split.c --- */
char					**ft_mini_split(t_minishell *ms, char const *s, char c);
int						word_counter(const char *s, char c);
char					*str_maker(const char *s, int start, int end);

/* --- sort_env.c ---*/
void					sort_utils(t_minishell *ms, t_env *current, t_env *temp,
							t_env *next);
void					sort_export(t_minishell *ms);

/****************************************************************/
/*																*/
/*							Signals								*/
/*																*/
/****************************************************************/

/* --- signals.c --- */
void					signal_handler(int signum);
void					signal_init(void);
#endif