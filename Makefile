GREEN  = \033[0;32m
YELLOW = \033[0;33m
RED    = \033[0;31m
RESET  = \033[0m

CXX      = clang++
CXXFLAGS = -Wall -Wextra -Werror -std=c++26 -pthread -O3 -I./includes

NAME     = rbuf

SRC_DIR  = src
OBJ_DIR  = obj

SRC_FILES = $(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ_FILES)
	@printf "$(GREEN)$(NAME) built successfully!\n$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)Compiling $<...$(RESET)\n"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clangd: clean
	@which bear >/dev/null 2>&1 || { printf "$(RED)Error: bear is not installed.\n$(RESET)"; exit 1; }
	@printf "$(GREEN)Generating compile_commands.json...\n$(RESET)"
	@bear -- $(MAKE) all

clean:
	@printf "$(RED)Cleaning object files...\n$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@printf "$(RED)Removing binary...\n$(RESET)"
	@rm -f $(NAME)
	@rm -f compile_commands.json

re: fclean all

.PHONY: all clean fclean re clangd
