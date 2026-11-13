##
## C/SDL3 Raycaster Makefile
##

SRC		=	./src/main.c	\
			./src/color.c	\
			./src/map.c

NAME	=	raycaster

CC		=	gcc

RM		=	rm -f

OBJ		=	$(SRC:.c=.o)

CFLAGS	=	-O2 -W -Wall -Wextra -Werror
CFLAGS	+=	-I./hds/

LDLIBS	=	-lSDL3 -lm

all		:	$(NAME)

$(OBJ)		:	./hds/raycaster.h

$(NAME)	:	$(OBJ)
			$(CC) $(LDFLAGS) -o $(NAME) $(OBJ) $(LDLIBS)

clean	:
			$(RM) $(OBJ)

fclean	:	clean
			$(RM) $(NAME)

re		:	fclean all

debug	:	CFLAGS	+=	-g -O1 -fsanitize=address -fno-omit-frame-pointer
debug	:	LDFLAGS	+=	-fsanitize=address
debug	:	fclean
			$(CC) $(CFLAGS) $(LDFLAGS) -o $(NAME) $(SRC) $(LDLIBS)

.PHONY	:	all clean fclean re debug
