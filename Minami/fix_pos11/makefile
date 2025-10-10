# ほぼコピペ

CC	  = g++
CFLAGS    = -std=c++17
INCLUDE   = -I./inc -I C:\Users\sisim\vcpkg\installed\x64-mingw-dynamic\include
LIBLINK = -L C:\Users\sisim\vcpkg\installed\x64-mingw-dynamic\lib	
LIBLARY = -llmdb  -lcpr -lcurl -lssl -lcrypto -lws2_32 -lcrypt32 -lwldap32

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
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBLINK) -fopenmp -o $@ $^ -O4 $(LIBLARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
# 	${CC} $(CFLAGS) $(INCLUDE) -c $< -o $@ $(ASAN_FLAGS)
	${CC} $(CFLAGS) $(INCLUDE) $(LIBLINK) -fopenmp -c $< -o $@ -O4 $(LIBLARY)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

