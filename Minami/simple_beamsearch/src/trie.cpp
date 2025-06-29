#include "../inc/all.hpp"

TrieNode* TrieNode::getChild(int idx) {
    return this->next[idx];
}

void TrieNode::setChild(int idx, TrieNode *child) {
    this->next[idx] = child;
}

Trie::Trie(int array_size, int type): array_size(array_size), type(type), _size(0){
    this->top = new TrieNode(0, 1);
}

bool Trie::insert(uint8_t *array) {
    TrieNode *tmp = top;
    bool res = false;
    /*rep (i, N * N) {
        if (tmp->getChild(array[i]) == NULL) {
            tmp->setChild(array[i], new TrieNode(i + 1, 0));
            res = true;
        }
        tmp = tmp->getChild(array[i]);
    }*/
    Seg<uint8_t> is_ok = createSegTree(N * N / 2, (uint8_t)0, seg_ope);
    uint8_t cnt[N * N / 2];
    int max_val = 0;
    rep (i, N * N / 2) {
        cnt[i] = 0; 
        is_ok.set(i, 1);
    }
    rep (i, N * N) {
        int idx = is_ok.getInterval(0, array[i] + 1) - 1;
        if (cnt[array[i]] == 0) max_val++;
        cnt[array[i]]++;
        if (cnt[array[i]] == 2) is_ok.set(array[i], 0);
        if (tmp->getChild(idx) == NULL) {
            tmp->setChild(idx, new TrieNode(i + 1, is_ok.getInterval(0, max_val + 1)));
            res = true;
        }
        tmp = tmp->getChild(idx);
    }
    free(is_ok.array);
    this->_size += res;
    return res;
}

bool Trie::find(uint8_t *array) {
    TrieNode *tmp = top;
    rep (i, N * N) {
        if (tmp->getChild(array[i]) == NULL) return false;
        tmp = tmp->getChild(array[i]);
    }
    return true;
}

size_t Trie::size() {
    return this->_size;
}

TrieNode::TrieNode(int depth, int next_size): depth(depth){
    this->next = new TrieNode*[next_size];
    rep (i, next_size) next[i] = NULL;
}

uint8_t seg_ope(uint8_t x, uint8_t y) {
    return x + y;
}