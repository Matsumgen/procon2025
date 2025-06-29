#ifndef TRIE_HPP
#define TRIE_HPP 0

#include "main.hpp"

class TrieNode {
    private:
        TrieNode **next;
        // map<uint8_t, TrieNode*> next;
        int depth;

    public:
        TrieNode(int depth, int next_size);
        TrieNode *getChild(int idx);
        void setChild(int idx, TrieNode *child);
};

class Trie {
    private:
        int array_size;
        int type;
        size_t _size;
        TrieNode *top;

    public:
        Trie(int array_size, int type);
        bool insert(uint8_t *array);
        bool find(uint8_t *array);
        size_t size();
};

uint8_t seg_ope(uint8_t x, uint8_t y);
#endif