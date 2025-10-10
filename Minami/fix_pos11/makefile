# ほぼコピペ

CC	  = g++
CFLAGS    = -std=c++17
INCLUDE   = -I./inc -I vcpkg_installed/x64-linux/include
LIBLINK = -L vcpkg_installed/x64-linux/lib/
LIBLARY = -llmdb

TARGET    = exe
SRCDIR    = ./src
OBJDIR    = ./obj
SUFFIX    = .cpp
ASAN_FLAGS = -fsanitize=address

SRCS      = $(wildcard $(SRCDIR)/*$(SUFFIX))
OBJS      = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:$(SUFFIX)=.o)))

all: $(TARGET)

# -include $(addprefix $(DEPDIR)/, $(notdir $(OBJS:.o=.Po)))

$(TARGET): $(OBJS)
# 	$(CC) -o $@ $^ $(ASAN_FLAGS) 
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBLINK) -o $@ $^ -O4 $(LIBLARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
# 	${CC} $(CFLAGS) $(INCLUDE) -c $< -o $@ $(ASAN_FLAGS)
	${CC} $(CFLAGS) $(INCLUDE) $(LIBLINK) -c $< -o $@ -O4 $(LIBLARY)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

