#include "../inc/all.hpp"

State::State() : x_hosei(0), y_hosei(0), rotate_hosei(0), progress(0), log(v_pair_ii(0)), score(0), end_flag(false), last_type(VERTICAL) {
}


/**
 * 終了したかを判定するメソッド
 * 2x2の盤面に行きつく(end_flag = true)ことも終了とみなす
 */
bool State::isEnd() {
    if (end_flag) return true;
    rep (i, this->f.size * this->f.size / 2) {
        if (manhattan(this->f.getEntPos(i, 0), this->f.getEntPos(i, 1)) != 1) return false;
    }
    return true;
}


/**
 * 評価を取得するメソッド
 */
int State::getScore() {
    return this->score;
}


/**
 * 指定したタイプで揃える際の次の状態の個数を取得するメソッド
 * できないタイプが指定されたら0を返す
 * 引数:
 *   int type : ペアの揃え方(progress = 0のときは0とする)
 */
int State::getNextCount(int type) {
    if (this->progress == 0) return 4 * (type == 0);
    if (this->last_type == HORIZON_0 && type != HORIZON_1) return 0;
    if ((this->progress == 1 || this->progress == this->f.size + 1) && type == HORIZON_1) return 0;
    Pos base_pos = this->getBasePos(type);
    Pos last_pos = this->getLastPos(type);
    Pos target_pos = this->f.getPairPos(base_pos);
    // printf("%d: %d, %d, (%d, %d), (%d, %d), (%d, %d)\n", this->progress, type, last_type, base_pos.x, base_pos.y, target_pos.x, target_pos.y, last_pos.x, last_pos.y);
    if (target_pos == last_pos) return 1;
    return BFS_result::getOperationCount(this->f.size, this->progress, target_pos, type);
}


/**
 * 今の状態を次の状態に進めるメソッド
 * 引数:
 *   int type : ペアの揃えるタイプ
 *   int idx : 複数ある候補のうち何番目の状態に進めるか
 */
void State::moveNextState(int type, int idx) {
    v_ope opes = this->getOperation(type, idx);
    for (Ope &ope : opes) this->f.rotate(ope);
    this->last_type = type;

    if (this->progress == 0) {
        // 全体回転の時はスコアは変えずにrotate_hoseiのみ変える
        this->rotate_hosei += opes.size();
        this->rotate_hosei &= 3;
    } else {
        // 操作した列の分だけスコアを減らす
        this->score -= (int)opes.size();
    }

    this->progress++;
    if (this->progress == this->f.size * 2 - 1) {
        // 右端の一番下の部分(あるサイズにおいて一番最後に揃えるペア)を揃えた場合の処理
        if (this->f.size == 4) {
            this->end_flag = true;
        } else {
            this->f.toSmall(0, 2, this->f.size - 2);
            this->progress = 0;
            this->x_hosei += 2 * (this->rotate_hosei == 1 || this->rotate_hosei == 2);
            this->y_hosei += 2 * (this->rotate_hosei <= 1);
        }
    }
    this->log.push_back(make_pair(type, idx));
}


/**
 * クローンを取得するメソッド
 */
void State::getClone(State *out) {
    out->x_hosei = this->x_hosei;
    out->y_hosei = this->y_hosei;
    out->rotate_hosei = this->rotate_hosei;
    this->f.getClone(&out->f);
    out->progress = this->progress;
    out->score = this->getScore();
    out->end_flag = this->end_flag;
    out->log = this->log;
    out->last_type = this->last_type;
}


/**
 * 答えの操作列を取得するメソッド
 * 意図的ではないが再帰になった
 * 引数:
 *   v_pair_ii &ans_log : 操作の履歴(各状態で選んだtypeとindex)
 *   int idx : ans_logの何番目か
 *   v_ope &out : 答えの操作列が入る配列
 */
void State::getAnswer(v_pair_ii &ans_log, int idx, v_ope &out) {
    if (this->progress != 0) {
        v_ope ope_list = this->getOperation(ans_log[idx].first, ans_log[idx].second);
        for (Ope &ope : ope_list) {
            Ope tmp = rotateOpe(ope, this->f.size, this->rotate_hosei);
            tmp.x += this->x_hosei;
            tmp.y += this->y_hosei;
            out.push_back(tmp);
        }
    } else {
        cout << ans_log[idx].second << " " << this->rotate_hosei << " " << this->x_hosei << " " << this->y_hosei << endl;;
    }

    if (idx + 1 < ans_log.size()) {
        this->moveNextState(ans_log[idx].first, ans_log[idx].second);
        this->getAnswer(ans_log, idx + 1, out);
    }
}


/**
 * 次の状態に移行する際に行う操作を取得するメソッド
 * 引数 : 
 *   int type : ペアの揃えるタイプ
 *   int idx : 複数ある候補のうち何番目の状態に進めるか
 */
v_ope State::getOperation(int type, int idx) {
    v_ope res;
    if (this->progress == 0) {
        // 全体回転の時はidx回回転させる
        rep (i, idx) res.push_back(Ope(0, 0, this->f.size));
    } else {
        Pos base_pos = this->getBasePos(type);
        Pos target_pos = this->f.getPairPos(base_pos);
        Pos last_pos = this->getLastPos(type);

        // 既にゴールの位置にある場合は空の配列を返す
        if (target_pos == last_pos) {
            return res;
        }

        // bfs_resultから取得
        v_ope tmp = BFS_result::getOperation(this->f.size, progress, target_pos, type, idx);
        for (Ope &ope : tmp) {
            res.push_back(ope);
            // printf("(%d, %d, %d) ", ope.x, ope.y, ope.n);
        }
        // cout << endl;
    }
    return res;
}


/**
 * 基準となる位置を取得するメソッド
 * 基準となる位置に存在するエンティティが揃えられる
 */
Pos State::getBasePos(int type) {
    if (this->progress >= 1 && this->progress <= this->f.size) {
        // 上端を揃えている場合
        if (type == HORIZON_1) {
            return Pos(this->progress - 2, 1);
        } else {
            return Pos(this->progress - 1, 0);
        }
    } else {
        // 右端を揃えている場合
        if (type == HORIZON_1) {
            return Pos(this->f.size - 2, 2 + (this->progress - this->f.size - 1) - 1);
        } else {
            return Pos(this->f.size - 1, 2 + (this->progress - this->f.size - 1));
        }
    }
}


/**
 * 目標の位置を取得するメソッド
 * 目標の位置にエンティティがくるとペアになる
 */
Pos State::getLastPos(int type) {
    Pos base_pos = this->getBasePos(type);
    if (this->progress >= 1 && this->progress <= this->f.size) {
        // 上端を揃えている場合
        if (type == VERTICAL) {
            return base_pos + Pos(0, 1);
        } else {
            return base_pos + Pos(1, 0);
        }
    } else {
        // 右端を揃えている場合
        if (type == VERTICAL) {
            return base_pos + Pos(-1, 0);
        } else {
            return base_pos + Pos(0, 1);
        }
    }
}