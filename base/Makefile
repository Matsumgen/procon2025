CC	  = g++
CFLAGS    = -std=c++17
INCLUDE   = -I./inc -I./lib/lmdb/include
LIBRARY   = ./lib/lmdb/lib/liblmdb.a

TARGET    = exe
SRCDIR    = ./src
OBJDIR    = ./obj
SUFFIX    = .cpp

SRCS      = $(wildcard $(SRCDIR)/*$(SUFFIX))
OBJS      = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:$(SUFFIX)=.o)))

all: $(TARGET)

-include $(addprefix $(DEPDIR)/, $(notdir $(OBJS:.o=.Po)))

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBRARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	${CC} $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

