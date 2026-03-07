# コンパイラとフラグ
CC       = g++
NVCC     = nvcc
FLAGS    = -std=c++17
CFLAGS   = -Wall -Wextra 
#ASAN_FLAGS = -fsanitize=address
ASAN_FLAGS = 
DEBUG    ?= 0

ifeq ($(DEBUG), 1)
    FLAGS += -g -O0
    CFLAGS += $(ASAN_FLAGS)
    LDFLAGS += $(ASAN_FLAGS)
else
    FLAGS += -O3
endif
CUFLAGS  = $(FLAGS) -Xcompiler "$(CFLAGS)" -lineinfo

# インクルードとライブラリ
#INCLUDE  = -Iinc -Isrc -Ilib/lmdb/include
INCLUDE_OMP = -Xpreprocessor -fopenmp -I/opt/homebrew/opt/libomp/include
#LIBRARY  = lib/lmdb/lib/liblmdb.a
LIBRARY_OMP = -L/opt/homebrew/opt/libomp/lib -lgomp
INCLUDE  = -Iinc -Isrc
#INCLUDE_OMP =
LIBRARY  =
#LIBRARY_OMP =

# ディレクトリ
SRCDIR  = src
MAINDIR = main
TESTDIR = test
OBJDIR  = obj
DEPDIR  = dep

# ファイル拡張子
SUFFIX_CPP = .cpp
SUFFIX_CU  = .cu

# 出力ファイル
MAIN_TARGET = exe
TEST_TARGET = test.exe

# ソースファイル
SRCS_CPP = $(wildcard $(SRCDIR)/*$(SUFFIX_CPP))
OBJS_CPP = $(addprefix $(OBJDIR)/, $(notdir $(SRCS_CPP:$(SUFFIX_CPP)=.o)))

SRCS_CU  = $(wildcard $(SRCDIR)/*$(SUFFIX_CU))
OBJS_CU  = $(addprefix $(OBJDIR)/, $(notdir $(SRCS_CU:$(SUFFIX_CU)=.o)))

OBJS_ALL = $(OBJS_CPP) $(OBJS_CU)

# 本番用
MAIN_SRC   = $(MAINDIR)/main.cpp
MAIN_OBJ   = $(OBJDIR)/main.o

# テスト用
TEST_SRC  = $(TESTDIR)/test.cpp
TEST_OBJ  = $(OBJDIR)/test.o

# ==============================
# ターゲット
all: $(MAIN_TARGET)

$(MAIN_TARGET): $(OBJS_ALL) $(MAIN_OBJ)
	$(NVCC) -o $@ $^ $(LDFLAGS) $(LIBRARY) $(LIBRARY_OMP)

test: $(TEST_TARGET)

$(TEST_TARGET): $(OBJS_ALL) $(TEST_OBJ)
	$(NVCC) -o $@ $^ $(LDFLAGS) $(LIBRARY) $(LIBRARY_OMP)

# ==============================
# オブジェクトファイルルール
# .cpp
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(FLAGS) $(CFLAGS) $(INCLUDE) $(INCLUDE_OMP) -MMD -MP -c $< -o $@

# .cu
$(OBJDIR)/%.o: $(SRCDIR)/%.cu
	@mkdir -p $(OBJDIR)
	$(NVCC) $(CUFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

# main.cpp
$(OBJDIR)/main.o: $(MAIN_SRC)
	@mkdir -p $(OBJDIR)
	$(CC) $(FLAGS) $(CFLAGS) $(INCLUDE) $(INCLUDE_OMP) -MMD -MP -c $< -o $@

# test.cpp
$(OBJDIR)/test.o: $(TEST_SRC)
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDE) $(INCLUDE_OMP) -MMD -MP -c $< -o $@

# 依存関係
-include $(wildcard $(DEPDIR)/*.Po)

# ==============================
clean:
	rm -f $(OBJDIR)/*.o $(DEPDIR)/*.Po

fclean: clean
	rm -f $(MAIN_TARGET) $(TEST_TARGET)

re: fclean all

