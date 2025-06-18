NAME = webserv

CXX = c++

# CXXFLAGS = -Wall -Wextra -Werror -std=c++98

RM = rm -rf

SRC =  main.cpp confugParser.cpp route.cpp Server.cpp Multiplexer.cpp Client.cpp HttpRequest.cpp Utils.cpp \

OBJ = $(SRC:.cpp=.o)

all:$(NAME)

$(NAME):$(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean:clean
	$(RM) $(NAME)
	
re:fclean all

.SECONDARY: $(OBJ)