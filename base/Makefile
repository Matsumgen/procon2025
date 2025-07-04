# コンパイラとフラグ
CC       = g++
CFLAGS   = -std=c++17 -Wall -Wextra
DEBUG    ?= 0

ifeq ($(DEBUG), 1)
    CFLAGS += -g -O2
else
    CFLAGS += -O3
endif

INCLUDE  = -Iinc -Isrc -Ilib/lmdb/include
LIBRARY  = lib/lmdb/lib/liblmdb.a

# ディレクトリ
SRCDIR  = src
MAINDIR = main
TESTDIR = test
OBJDIR  = obj
DEPDIR  = dep

# ファイル拡張子
SUFFIX  = .cpp

# 出力ファイル
MAIN_TARGET = exe
TEST_TARGET = test.exe

# ソースファイル（共通コード）
SRCS = $(wildcard $(SRCDIR)/*$(SUFFIX))
OBJS = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:$(SUFFIX)=.o)))

# 本番用
MAIN_SRC   = $(MAINDIR)/main.cpp
MAIN_OBJ   = $(OBJDIR)/main.o
MAIN_DEP   = $(DEPDIR)/main.Po

# テスト用
TEST_SRC  = $(TESTDIR)/test.cpp
TEST_OBJ  = $(OBJDIR)/test.o
TEST_DEP  = $(DEPDIR)/test.Po

# 本番ターゲット
all: $(MAIN_TARGET)

$(MAIN_TARGET): $(OBJS) $(MAIN_OBJ)
	$(CC) -o $@ $^ $(LIBRARY)

# テストターゲット
test: $(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_OBJ)
	$(CC) -o $@ $^ $(LIBRARY)

# 個々のオブジェクトファイルルール
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

$(OBJDIR)/main.o: $(MAIN_SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

$(OBJDIR)/test.o: $(TEST_SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

# 依存関係を含める
-include $(wildcard $(DEPDIR)/*.Po)

clean:
	rm -f $(OBJDIR)/*.o $(DEPDIR)/*.Po

fclean: clean
	rm -f $(MAIN_TARGET) $(TEST_TARGET)

re: fclean all

