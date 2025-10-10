CC	  = g++
CFLAGS    = -std=c++17 -O4 -fexec-charset=CP932
INCLUDE   = -I./inc -I C:\Users\sisim\vcpkg\installed\x64-mingw-dynamic\include
LIBLINK   = -L C:\Users\sisim\vcpkg\installed\x64-mingw-dynamic\lib
LIBLARY   = -llmdb -lcpr -lcurl -lssl -lcrypto -lws2_32 -lcrypt32 -lwldap32

TARGET    = exe
SRCDIR    = ./src
OBJDIR    = ./obj
SUFFIX    = .cpp

SRCS      = $(wildcard $(SRCDIR)/*$(SUFFIX))
OBJS      = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:$(SUFFIX)=.o)))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBLINK) -o $@ $^ $(LIBLARY)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all
