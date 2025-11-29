NAME = ircserv

SRC = $(wildcard *.cpp)
#SRC = main.cpp etc

CC = c++
CFLAGS = -Werror -Wextra -Wall -std=c++98 -Ihdr

#SRC_DIR = src
SRC_DIR = .
OBJ_DIR = obj

OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

all: $(NAME) run

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@
	@echo ./$(NAME)
# 	@./$(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all re clean fclean

