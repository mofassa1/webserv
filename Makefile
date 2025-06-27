NAME = webserv

CXX = c++

# CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CLEAR = clear
RM = rm -rf

SRC =  main.cpp confugParser.cpp route.cpp Server.cpp Multiplexer.cpp Client.cpp HttpRequest.cpp Utils.cpp CGI.cpp HandleRequest.cpp HandleResponse.cpp HTTP_Methods.cpp  \

OBJ = $(SRC:.cpp=.o)

all:$(NAME)

$(NAME):$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean:clean
	$(RM) $(NAME)
	
re:fclean all


build:
	make re
	make clean
	clear

.SECONDARY: $(OBJ)