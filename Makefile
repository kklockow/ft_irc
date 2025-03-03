CXX = c++
NAME = ircserv
CXXFLAGS = -g3 -std=c++17 -Wall -Werror -Wextra

SOURCE = main.cpp Server.cpp utils.cpp Client.cpp

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