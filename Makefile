SRCS = 	src/main.cpp \
		src/Parsing.cpp \
		src/Request.cpp \
		src/Server.cpp \
		src/Response.cpp \
		src/MimeType.cpp

OBJS = $(SRCS:.cpp=.o)

NAME = webserv

HEADERS = -I inc/

C++FLAGS = -Wall -Wextra -Werror -std=c++98

.cpp.o:
	c++ $(C++FLAGS) $(HEADERS) -c $< -o $(<:.cpp=.o)

$(NAME): $(OBJS)
	c++ $(C++FLAGS) $(HEADERS) $(OBJS) -o $(NAME)

all: $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re