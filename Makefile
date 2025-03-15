CXX = c++
NAME = ircserv
CXXFLAGS = -fsanitize=address -g3 -std=c++17 -Wall -Werror -Wextra

SOURCE = main.cpp Server/Server.cpp Server/Server_commands.cpp Server/Server_utils.cpp Utility/utils.cpp Client/Client.cpp Channel/Channel.cpp OP_cmds.cpp

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