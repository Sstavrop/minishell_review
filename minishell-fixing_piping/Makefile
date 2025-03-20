NAME							= minishell

# Compiler and Cflags
CC								= cc
CFLAGS							= -Wall -Wextra -Werror -g
LDFLAGS							= -lreadline

# Directories
SRC_DIR							= ./sources/
OBJ_DIR							= ./objects/
INC_DIR							= ./includes/
LIBFT_DIR						= ./libft

# Sources
SRC								=	main.c \
									builtins/cd.c \
									builtins/echo.c \
									builtins/env.c \
									builtins/exit.c \
									builtins/export.c \
                                    builtins/pwd.c \
									builtins/unset.c \
									execution/execute_builtins.c \
									execution/execute_externals.c \
									execution/handle_pipes.c \
									execution/pather.c \
									parsing/expand_variables.c \
									parsing/handle_heredoc.c \
									parsing/handle_redirection.c \
									parsing/heredoc_utils.c \
                                    parsing/parser_utils.c \
                                    parsing/parsing_tokens.c \
									signals/signals.c \
									tokenizing/handle_quotes.c \
                                    tokenizing/tokenizing_utils.c \
                                    tokenizing/tokenizing_utils2.c \
                                    tokenizing/tokenizing.c \
                                    tokenizing/token_list.c \
									utils/ft_mini_split.c \
                                    utils/prompt.c \
									utils/sort_env.c \
                                    utils/free_list.c \

SRCS							= $(addprefix $(SRC_DIR), $(SRC))
OBJ								= $(SRC:.c=.o)
OBJS							= $(addprefix $(OBJ_DIR)/, $(OBJ))
INC								= -I $(INC_DIR) -I $(LIBFT_DIR)

# Libft
LIBFT							= $(LIBFT_DIR)/libft.a

# All Rule
all:							$(OBJ_DIR) $(LIBFT) $(NAME)
								@echo "\033[4;36m[Program is ready]\033[0m"

# Object Directory Rule
$(OBJ_DIR):
								mkdir -p $(OBJ_DIR)
								mkdir -p $(OBJ_DIR)/utils
								mkdir -p $(OBJ_DIR)/builtins
								mkdir -p $(OBJ_DIR)/signals
								mkdir -p $(OBJ_DIR)/execution
								mkdir -p $(OBJ_DIR)/parsing
								mkdir -p $(OBJ_DIR)/tokenizing

# Objects rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
								$(CC) $(CFLAGS) -c $< -o $@ $(INC)

# Project file rule
$(NAME):						$(OBJS)
								$(CC) $(CFLAGS) $(OBJS) -o $@ $(INC) $(LIBFT) $(LDFLAGS)
								@echo "\033[4;36m[minishell created]\033[0m"

# Libft rule
$(LIBFT):
								make -C $(LIBFT_DIR)
								@echo "\033[4;36m[Libft created]\033[0m"

# Clean up build files rule
clean:
								rm -rf $(OBJ_DIR)
								make -C $(LIBFT_DIR) clean

# Remove program executable
fclean:							clean
								rm -f $(NAME)
								make -C $(LIBFT_DIR) fclean

# Clean + remove executable
re:								fclean all

.PHONY: all re clean fclean
