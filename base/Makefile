CC	  = g++
CFLAGS    =
INCLUDE   = -I./inc

TARGET    = exe
SRCDIR    = ./src
OBJDIR    = ./obj
SUFFIX    = .cpp

SRCS      = $(wildcard $(SRCDIR)/*$(SUFFIX))
OBJS      = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:$(SUFFIX)=.o)))

all: $(TARGET)

-include $(addprefix $(DEPDIR)/, $(notdir $(OBJS:.o=.Po)))

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	${CC} $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

