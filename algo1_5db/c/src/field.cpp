//#include "../inc/all.hpp"
#include <field.hpp>
#include <utilities.hpp>

Pos::Pos() {
}

Pos::Pos(short x, short y) : x(x), y(y) {
}

Pos Pos::operator + (const Pos &other) const {
    return Pos(this->x + other.x, this->y + other.y);
}

bool Pos::operator == (const Pos &other) const {
    return this->x == other.x && this->y == other.y;
}

int Pos::toInt(int size) {
    return this->x + this->y * size;
}

Ent::Ent() {
}

Ent::Ent(short val, short num) : val(val), num(num) {
}

Ope::Ope() {
}

Ope::Ope(short x, short y, short n) : x(x), y(y), n(n) {
}

bool Ope::operator < (const Ope &other) const {
    return this->x < other.x || (this->x == other.x && this->y < other.y) || (this->x == other.x && this->y == other.y && this->n < other.n);
}

Field::Field() {
}

Field::Field(short size) : size(size) {
    this->ent_mem = new Ent[this->size * this->size];
    this->pos_mem = new Pos[this->size * this->size];
}

Field::Field(short size, Ent *ent_mem, Pos *pos_mem) : size(size), ent_mem(ent_mem), pos_mem(pos_mem) {
}

Ent& Field::getEnt(int y, int x) {
    return this->ent_mem[y * this->size + x];
}

Pos& Field::getEntPos(int val, int num) {
    return this->pos_mem[val * 2 + num];
}

Pos Field::getPairPos(const Pos &pos) {
    return getPairPos(this->getEnt(pos.y, pos.x));
}

Pos Field::getPairPos(const Ent &ent) {
    return this->getEntPos(ent.val, (ent.num + 1) & 1);
}

void Field::getClone(Field *out) {
    out->size = this->size;
    memcpy(out->ent_mem, this->ent_mem, this->size * this->size * sizeof(Ent));
    memcpy(out->pos_mem, this->pos_mem, this->size * this->size * sizeof(Pos));
}

void Field::rotate(Ope ope) {
    int a = ope.n >> 1;
    int b = ope.n & 1;
    Ent buf;
    int h1, w1, h2, w2;
    //動かす盤面を4等分して動かす(奇数の時は真ん中は除く)
    for(h1 = 0; h1 < a; h1++){
        h2 = ope.n - h1 - 1;
        for(w1 = 0; w1 < a; w1++){
            w2 = ope.n - w1 - 1;
            buf = this->getEnt(ope.y + h1, ope.x + w1);
    
            int dy[4] = {h1, w2, h2, w1};
            int dx[4] = {w1, h1, w2, h2};
            rep (i, 4){
                Pos setting = (Pos){static_cast<uint8_t>(ope.x + dx[i]), static_cast<uint8_t>(ope.y + dy[i])};
                this->getEnt(setting.y, setting.x) = i == 3 ? buf : this->getEnt(ope.y + dy[i + 1], ope.x + dx[i + 1]);
                auto& ent = this->getEnt(setting.y, setting.x);
                this->getEntPos(ent.val, ent.num) = setting;
                /* this->getEntPos(this->getEnt(setting.y, setting.x).val, this->getEnt(setting.y, setting.x).num) = setting; */
            }
        }
    }
    //奇数の時の真ん中を動かす
    if(b == 1){
        int mw = ope.x + a, mh = ope.y + a;
        for(int i = 0; i < a; i++){
            w1 = ope.x + i;
            w2 = ope.x + ope.n - i - 1;
            h1 = ope.y + i;
            h2 = ope.y + ope.n - i - 1;
            buf = this->getEnt(h1, mw);
    
            int dy[4] = {h1, mh, h2, mh};
            int dx[4] = {mw, w1, mw, w2};
            rep (j, 4){
                Pos setting = (Pos){static_cast<uint8_t>(dx[j]), static_cast<uint8_t>(dy[j])};
                this->getEnt(setting.y, setting.x) = j == 3 ? buf : this->getEnt(dy[j + 1], dx[j + 1]);
                auto& ent = this->getEnt(setting.y, setting.x);
                this->getEntPos(ent.val, ent.num) = setting;
                /* this->getEntPos(this->getEnt(setting.y, setting.x).val, this->getEnt(setting.y, setting.x).num) = setting; */
            }
        }
    }
}

void Field::toSmall(int x, int y, int next_size) {
    if ((next_size & 1) || x + next_size > this->size || y + next_size > this->size) {
        cout << "Logic Error\nnext_size is invalid" << endl;
        exit(1);
    }
    int p_size = this->size;
    Ent *tmp_ptr = this->ent_mem + y * p_size + x;
    this->size = next_size;
    rep (i, next_size) {
        memmove(this->ent_mem + next_size * i, tmp_ptr + p_size * i, next_size * sizeof(Ent));
    }
    // cout << endl;
    // this->printField();
    // cout << endl;
    this->reallocation();
}

void Field::reallocation() {
    int cnt = 0;
    int max_cnt = this->size * this->size / 2;
    int new_val[288];
    rep (i, 288) new_val[i] = -1;
    rep (i, this->size) rep (j, this->size) {
        Ent &tmp = this->getEnt(i, j);
        if (new_val[tmp.val] == -1) {
            if (cnt == max_cnt) {
                cout << "Logic Error\ncnt is too big." << endl;
                exit(1);
            }
            new_val[tmp.val] = cnt;
            cnt++;
        }
        tmp.val = new_val[tmp.val];
        this->getEntPos(tmp.val, tmp.num) = Pos(j, i);
    }
}

void Field::printField() {
    rep (i, this->size) rep (j, this->size) printf("%3d%c", this->getEnt(i, j).val, " \n"[j == this->size - 1]);
    /* cout << this->getEnt(i, j).val << " \n"[j == this->size - 1]; */
    cout << endl;
}

void Field::printEntPos() {
    rep (i, this->size * this->size / 2) printf("%d: (%d, %d), (%d, %d)\n", i, this->getEntPos(i, 0).y, this->getEntPos(i, 0).x, this->getEntPos(i, 1).y, this->getEntPos(i, 1).x);
    cout << endl;
}
