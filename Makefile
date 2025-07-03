# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lbenatar <lbenatar@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/03/16 14:46:31 by prambaud          #+#    #+#              #
#    Updated: 2025/06/03 09:07:09 by lbenatar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	webserv

CXX			=	c++

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 -g3

# directories
SRCS_DIR		=	srcs/
OBJS_DIR		=	.objs/
INCS_DIR		=	includes/

# controll codes
RESET		=	\033[0m
GREEN		=	\033[32m
YELLOW		=	\033[33m
BLUE		=	\033[34m
RED			=	\033[31m
UP			=	\033[A
CUT			=	\033[K

#source files
SRCS_FILES	=	main.cpp \
				sighandler.cpp \
				confParsing.cpp \
				serverSetUp.cpp \
				requestParsing.cpp \
				HTTPRequest.cpp \
				responseParsing.cpp \
				HTTPResponse.cpp \
				callCGI.cpp \
				CGIProcess.cpp \
				autoIndexManagement.cpp \
				serverManagement.cpp \
				requestParsUtils.cpp \
				methodeHandlerUtils.cpp \
				deleteHandler.cpp \
				postHandler.cpp \
				requestFullyReceivedCheck.cpp \
				methodeHandler.cpp \
				postMultipart.cpp \
				serverSetUpUtils.cpp

OBJS_FILES	=	$(SRCS_FILES:.cpp=.o)

#paths
SRCS			=	$(addprefix $(SRCS_DIR), $(SRCS_FILES))
OBJS			=	$(addprefix $(OBJS_DIR), $(OBJS_FILES))
DEPS			=	$(OBJS:.o=.d)

#all rule
all: $(NAME)

#compile the executable
$(NAME): $(OBJS)
	@echo "$(YELLOW)Compiling [$(NAME)]...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)Finished [$(NAME)]$(RESET)"

-include $(DEPS)

#compile objects
$(OBJS_DIR)%.o:$(SRCS_DIR)%.cpp
	@mkdir -p $(OBJS_DIR)
	@echo "$(YELLOW)Compiling [$@]...$(RESET)"
	@$(CXX) $(CXXFLAGS) -MMD -MP -I $(INCS_DIR) -o $@ -c $<
	@printf "$(UP)$(CUT)"
	@echo "$(GREEN)Finished [$@]$(RESET)"
	@printf "$(UP)$(CUT)"

#clean rule
clean:
	@if [ -d "$(OBJS_DIR)" ]; then \
	rm -rf $(OBJS_DIR); \
	echo "$(BLUE)Deleting all objects from /ex00...$(RESET)"; else \
	echo "No objects to remove from /ex00."; \
	fi;
	@if [ -d ".tmp" ]; \
	then rm -rf .tmp; \
	fi;

#fclean rule
fclean: clean
	@if [ -f "$(NAME)" ]; then \
	rm -f $(NAME); \
	echo "$(BLUE)Deleting $(NAME) from /ex00...$(RESET)"; else \
	echo "No Executable to remove from /ex00."; \
	fi;

#re rule
re: fclean all

#phony
.PHONY: all clean fclean re
