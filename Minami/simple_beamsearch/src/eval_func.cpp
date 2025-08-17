#include "../inc/all.hpp"

int getScore1(State &s){
    int score = 0;
    for (int i = 0; i < N; i += 2) for (int j = 0; j < N; j += 2) {
        bool a, b, c, d;
        a = s.getEnt(i, j).val == s.getEnt(i, j + 1).val;
        b = s.getEnt(i + 1, j).val == s.getEnt(i + 1, j + 1).val;
        c = s.getEnt(i, j).val == s.getEnt(i + 1, j).val;
        d = s.getEnt(i, j + 1).val == s.getEnt(i + 1, j + 1).val;
        if (a && b || c && d){
            score += 1000;
        } else {
            score += a || b || c || d;
        }
    }
    return score;
}

int getScore2(State &s){
    int res = 0;
    rep (i, N * N / 2){
        res += manhattan(s.getEntPos(i, 0), s.getEntPos(i, 1)) == 1;
    }
    return res;
}

int getScore3(State &s){
    int score = 0;
    for (int i = 0; i < N; i += 2) for (int j = 0; j < N; j += 2) {
        bool a, b, c, d;
        a = s.getEnt(i, j).val == s.getEnt(i, j + 1).val;
        b = s.getEnt(i + 1, j).val == s.getEnt(i + 1, j + 1).val;
        c = s.getEnt(i, j).val == s.getEnt(i + 1, j).val;
        d = s.getEnt(i, j + 1).val == s.getEnt(i + 1, j + 1).val;
        score += a + b + c + d;
    }
    return score;
}

int getScore4(State &s) {
    return getScore2(s) + getScore3(s);
}

int getScore5(State &s) {
    int score = 0;
    for (int i = 0; i < N; i += 2) for (int j = 0; j < N; j += 2) {
        bool a, b, c, d;
        a = s.getEnt(i, j).val == s.getEnt(i, j + 1).val;
        b = s.getEnt(i + 1, j).val == s.getEnt(i + 1, j + 1).val;
        c = s.getEnt(i, j).val == s.getEnt(i + 1, j).val;
        d = s.getEnt(i, j + 1).val == s.getEnt(i + 1, j + 1).val;
        if (a && b || c && d) {
            score += 3;
        } else {
            score += a | b | c | d;
        }
    }
    return score;
}

int getScore6(State &s) {
    int score1 = 0;
    int score2 = 0;
    Pos center = (Pos){N / 4, N / 4};
    for (int i = 0; i < N; i += 2) for (int j = 0; j < N; j += 2) {
        bool a, b, c, d;
        a = s.getEnt(i, j).val == s.getEnt(i, j + 1).val;
        b = s.getEnt(i + 1, j).val == s.getEnt(i + 1, j + 1).val;
        c = s.getEnt(i, j).val == s.getEnt(i + 1, j).val;
        d = s.getEnt(i, j + 1).val == s.getEnt(i + 1, j + 1).val;
        score1 += a + b + c + d;
        if (!(a && b || c && d)) {
            Pos p1 = (Pos){j / 2, i / 2};
            score2 += manhattan(p1, center);
        }
    }
    return score1 * 10000 - score2;
}

int getScore7(State &s){
    int res = 0;
    rep (i, N * N / 2){
        res += manhattan(s.getEntPos(i, 0), s.getEntPos(i, 1)) - 1;
    }
    return -res;
}

int getScore8(State &s){
    int res = 0;
    rep (i, N * N / 2){
        int d = manhattan(s.getEntPos(i, 0), s.getEntPos(i, 1));
        res += -2 * d + d * (d % 2);
    }
    return res;
}

int getScore9(State &s){
    int score1 = 0;
    int score2 = 0;
    rep (i, N * N / 2){
        Pos p1 = s.getEntPos(i, 0), p2 = s.getEntPos(i, 1);
        p1.x /= 4;
        p1.y /= 4;
        p2.x /= 4;
        p2.y /= 4;
        score1 += p1 == p2;
        score2 += manhattan(p1, p2);
    }
    return score1 * 1000 - score2;
}

int getScore10(State &s) {
    int n = s.size;
    int score = 0;
    int range[4] = {0, 0, n, n};  // (min_x, min_y), (max_x, max_y)
    int x, y;
    rep (i, s.size / 2) {
        y = range[1];
        for (x = range[0]; x < range[2] - 2; x++) {
            if (s.getEnt(y, x).val != s.getEnt(y + 1, x).val) return score;
            score++;
        }
        for (y = range[1]; y < range[3] - 2; y++) {
            if (s.getEnt(y, x).val != s.getEnt(y, x + 1).val) return score;
            score++;
        }
        if (s.getEnt(y, x + 1).val != s.getEnt(y + 1, x + 1).val) return score;
        score++;
        if (s.getEnt(y, x).val != s.getEnt(y + 1, x).val) return score;
        score++;

        range[1] += 2;
        range[2] -= 2;
    }
    return score;
}

int getScore11(State &s) {
    return getScore10(s) + getScore2(s);
}