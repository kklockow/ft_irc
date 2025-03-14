CXX = c++
NAME = ircserv
CXXFLAGS = -fsanitize=address -g3 -std=c++17 -Wall -Werror -Wextra

SOURCE = main.cpp Server.cpp utils.cpp Client.cpp Channel.cpp OP_cmds.cpp

OBJECTS = $(SOURCE:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJECTS)
	@$(CXX) -o $(NAME) $(OBJECTS) $(CXXFLAGS)

clean:
	@rm -f $(OBJECTS)

fclean:
	@rm -f  $(OBJECTS) $(NAME)

re: fclean all

.PHONY : all, clean, fclean, re