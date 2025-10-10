#include "../inc/all.hpp"

// fsdb::Routes State::tmp_route;

State::State() : x_hosei(0), y_hosei(0), rotate_hosei(0), progress(0), log(v_pair_ii(0)), score(0), end_flag(false), last_type(FLAT), ok_pair(0), ope_sum(0), pile_dir(HORIZON) {
}

bool State::isEnd() {
    if (end_flag) return true;
    rep (i, this->f.size * this->f.size / 2) {
        if (manhattan(this->f.getEntPos(i, 0), this->f.getEntPos(i, 1)) != 1) return false;
    }
    return true;
}

int State::getScore() {
    return this->score;
}

int State::getNextCount(int type) {
    if (this->progress == 0) return 4 * (type == 0);
    if (!isOKType(type)) return 0;
    if (type == 14) return 1;
    if (type == 15) {
        return route->getSize();
    }

    v_pos base_pos = this->getBasePos(type);
    // v_pos last_pos = this->getLastPos(type);
    v_pos target_pos(base_pos.size());
    rep (i, (int)base_pos.size()) {
        target_pos[i] = this->f.getPairPos(base_pos[i]);
        rep (j, (int)base_pos.size()) {
            if (target_pos[i] == base_pos[j]) {
                return 0;
            }
        }
    }
    return BFS_result::getOperationCount(this, target_pos, type);
}

void State::moveNextState(int type, int idx) {
    if (this->progress == 0) {
        v_ope opes = this->getOperation(type, idx);
        for (Ope &ope : opes) this->f.rotate(ope);
        this->rotate_hosei += opes.size();
        this->rotate_hosei &= 3;
        this->progress++;
        this->last_type = FLAT;
        this->pile_dir = HORIZON;
        this->last_pair_x = -1;
    } else {
        v_ope opes = this->getOperation(type, idx);
        for (Ope &ope : opes) this->f.rotate(ope);
        int new_pair_cnt;
        if (type <= 7) {
            new_pair_cnt = type <= 2 ? 1 : 2;
            this->progress += new_pair_cnt;
            // this->last_pair_x += new_pair_cnt;  // 要検討
            // this->last_pair_x = this->progress <= this->f.size ? (this->progress - 1) : (this->progress - 1 - this->f.size);  // 要検討
            this->last_pair_x = this->progress <= this->f.size ? (this->progress - 2) : (this->progress - 2 - this->f.size);  // 要検討
            this->last_type = (type == 1 || type == 5 || type == 7) ? OUTSIDE : FLAT;
            this->pile_dir = HORIZON;
        } else if (type <= 13) {
            new_pair_cnt = 1;
            this->progress += new_pair_cnt;
            int tmp_type = type - 8;
            if (tmp_type == 2) {
                this->last_type = LEFT;
            } else if (tmp_type == 4) {
                this->last_type = RIGHT;
            } else {
                this->last_type = FLAT;
            }
            this->pile_dir = VERTICAL;
        } else if (type == 14) {
            new_pair_cnt = 0;
            // this->last_pair_x = this->progress <= this->f.size ? (this->progress - 1) : (this->progress - 1 - this->f.size);
            this->last_pair_x = this->progress <= this->f.size ? (this->progress - 2) : (this->progress - 2 - this->f.size);
            this->last_type = FLAT;
            this->pile_dir = HORIZON;
        } else if (type == 15) {
            new_pair_cnt = 2;
            this->progress += new_pair_cnt;
            this->last_pair_x = this->progress <= this->f.size ? (this->progress - 2) : (this->progress - 2 - this->f.size);
            this->last_type = FLAT;
            this->pile_dir = HORIZON;
        }
        this->ok_pair += new_pair_cnt;
        this->ope_sum += (int)opes.size();
        this->score = -10000 * ope_sum / ok_pair;

        if (this->pile_dir == HORIZON && this->progress == this->f.size * 2 - 1) {
            if (this->f.size == 6) {
                this->end_flag = true;
                this->f.toSmall(0, 2, this->f.size - 2);
                this->x_hosei += 2 * (this->rotate_hosei == 1 || this->rotate_hosei == 2);
                this->y_hosei += 2 * (this->rotate_hosei <= 1);

                v_ope final_ope = fdb::getField4(this->f);
                // cout << final_ope.size() << " " << flush;
                this->ope_sum += (int)opes.size();
                this->ok_pair += 8;
                this->score = -10000 * this->ope_sum / this->ok_pair;
            } else {
                this->f.toSmall(0, 2, this->f.size - 2);
                this->progress = 0;
                this->x_hosei += 2 * (this->rotate_hosei == 1 || this->rotate_hosei == 2);
                this->y_hosei += 2 * (this->rotate_hosei <= 1);
            }
        }
    }
    this->log.push_back(make_pair(type, idx));
}

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
    out->ok_pair = this->ok_pair;
    out->ope_sum = this->ope_sum;
    out->pile_dir = this->pile_dir;
    out->last_pair_x = this->last_pair_x;
    out->route = this->route;
}

void State::getAnswer(v_pair_ii &ans_log, int idx, v_ope &out) {
    Ent tmp_ent_mem[this->f.size * this->f.size];
    Pos tmp_pos_mem[this->f.size * this->f.size];
    State tmp_s;
    tmp_s.f.ent_mem = tmp_ent_mem;
    tmp_s.f.pos_mem = tmp_pos_mem;

    this->getClone(&tmp_s);
    fsdb::Routes tmp_route;
    tmp_s.route = &tmp_route;
    rep (i, (int)ans_log.size()) {
        if (tmp_s.progress == 0) {
            // cout << ans_log[i].second << " " << tmp_s.rotate_hosei << " " << tmp_s.x_hosei << " " << tmp_s.y_hosei << endl;
        } else {
            if (ans_log[i].first == 15) {
                tmp_route = fsdb::getOperation(&tmp_s);
            }
            v_ope ope_list = tmp_s.getOperation(ans_log[i].first, ans_log[i].second);
            for (Ope &ope : ope_list) {
                Ope tmp = rotateOpe(ope, tmp_s.f.size, tmp_s.rotate_hosei);
                tmp.x += tmp_s.x_hosei;
                tmp.y += tmp_s.y_hosei;
                out.push_back(tmp);
            }    
        }
        tmp_s.moveNextState(ans_log[i].first, ans_log[i].second);
        if (tmp_s.f.size == 4) {
            v_ope final_ope = fdb::getField4(tmp_s.f);
            for (Ope &ope : final_ope) {
                Ope tmp = rotateOpe(ope, tmp_s.f.size, tmp_s.rotate_hosei);
                tmp.x += tmp_s.x_hosei;
                tmp.y += tmp_s.y_hosei;
                out.push_back(tmp);
            }
            return;
        }
    }
}

v_ope State::getOperation(int type, int idx) {
    v_ope res;
    if (this->progress == 0) {
        rep (i, idx) res.push_back(Ope(0, 0, this->f.size));
    } else {
        if (type == 14) {
            return this->getToHorizonOpe();
        } else if (type == 15) {
            return route->getOperation(idx);
        } else {
            v_pos base_pos = this->getBasePos(type);
            // v_pos last_pos = this->getLastPos(type);
            v_pos target_pos(base_pos.size());
            rep (i, (int)base_pos.size()) target_pos[i] = this->f.getPairPos(base_pos[i]);

            // printf("{(%d, %d), (%d, %d)}, {(%d, %d), (%d, %d)}\n", base_pos[0].x, base_pos[0].y, target_pos[0].x, target_pos[0].y, base_pos[1].x, base_pos[1].y, target_pos[1].x, target_pos[1].y);
            v_ope tmp = BFS_result::getOperation(this, target_pos, type, idx);
            for (Ope &ope : tmp) {
                res.push_back(ope);
                // printf("(%d, %d, %d) ", ope.x, ope.y, ope.n);
            }
        }
    }
    return res;
}

bool State::isOKType(int type) { 
    if (this->progress == 0) return false;
    if (this->progress == this->f.size * 2 - 1 && type != 14) return false;

    int tmp_progress, dir;
    if (this->progress >= this->f.size + 1) {
        tmp_progress = this->progress - (this->f.size + 1);
        dir = 1;
    } else {
        tmp_progress = this->progress - 1;
        dir = 0;
    }

    if (type <= 2) {
        if (this->pile_dir == VERTICAL) return false;
        if (dir == 0) {
            if ((tmp_progress == this->f.size - 3 && type == 1) || (tmp_progress == this->f.size - 2 && type != 1) || (tmp_progress == this->f.size - 1 && type != 2)) return false;
        } else {
            if ((tmp_progress == this->f.size - 5 && type == 1) || (tmp_progress == this->f.size - 4 && type != 1) || (tmp_progress == this->f.size - 3 && type != 2)) return false;
        }

        // if (this->last_type == FLAT && type == 2 || this->last_type == OUTSIDE && type <= 1) return false;
        if (type == 0) {
            return this->last_type == FLAT;
        } else if (type == 1) {
            return this->last_type == FLAT;
        } else {
            return this->last_type == OUTSIDE;
        }
    } else if (type <= 7) {
        if (this->pile_dir == VERTICAL) return false;
        if (this->f.size == 24) return false;
        type -= 3;
        // if (tmp_progress == 0 && type >= 3) return false;
        if (dir == 0) {
            // if (tmp_progress == this->f.size - 1 || (tmp_progress == this->f.size - 2 && (type == 1 || type == 2 || type == 4)) || (tmp_progress == this->f.size - 3 && (type == 0 || type == 1 || type == 3)) || (tmp_progress == this->f.size - 4 && (type == 2 || type == 4))) return false;
            if (tmp_progress == this->f.size - 1 || (tmp_progress == this->f.size - 2 && (type == 1 || type == 2 || type == 4)) || (tmp_progress == this->f.size - 3) || (tmp_progress == this->f.size - 4 && (type == 2 || type == 4))) return false;
        } else {
            // if (tmp_progress == this->f.size - 3 || (tmp_progress == this->f.size - 4 && (type == 1 || type == 2 || type == 4)) || (tmp_progress == this->f.size - 5 && (type == 0 || type == 1 || type == 3)) || (tmp_progress == this->f.size - 6 && (type == 2 || type == 4))) return false;
            if (tmp_progress == this->f.size - 3 || (tmp_progress == this->f.size - 4 && (type == 1 || type == 2 || type == 4)) || (tmp_progress == this->f.size - 5) || (tmp_progress == this->f.size - 6 && (type == 2 || type == 4))) return false;
        }

        // if (last_type == FLAT && type >= 3 || last_type == OUTSIDE && type <= 2) return false;
        if (type == 0) {
            return this->last_type == FLAT;
        } else if (type == 1) {
            return this->last_type == FLAT;
        } else if (type == 2) {
            return this->last_type == FLAT;
        } else if (type == 3) {
            return this->last_type == OUTSIDE;
        } else {
            return this->last_type == OUTSIDE;
        }
    } else if (type <= 13) {
        type -= 8;
        if (tmp_progress == this->f.size - 1 - 2 * dir && (type == 2 || type == 4)) return false;
        int x = this->last_pair_x - 1;
        int y = tmp_progress - x;
        if (type == 3 || type == 5) y--;
        
        int w = this->f.size - 2 * dir;
        int max_y = this->f.size - 1;
        if (type >= 2) max_y--;
        if (!(x >= 0 && x < w - 1 && y >= 2 && y <= max_y)) return false;

        if (type == 0) {
            return this->last_type == FLAT;
        } else if (type == 1) {
            return this->last_type == FLAT;
        } else if (type == 2) {
            return x != 0 && this->last_type == FLAT; 
        } else if (type == 3) {
            return this->last_type == RIGHT; 
        } else if (type == 4) {
            return x != w - 2 && this->last_type == FLAT;
        } else {
            return this->last_type == LEFT;
        }
    } else if (type == 14) {
        return this->pile_dir == VERTICAL && this->last_type == FLAT;
    } else {
        return this->f.size <= 16 && this->pile_dir == HORIZON && this->last_type == FLAT;
    }
    return true;
}

v_pos State::getBasePos(int type) {
    v_pos res;
    int tmp_progress, dir;
    if (this->progress >= this->f.size + 1) {
        tmp_progress = this->progress - (this->f.size + 1);
        dir = 1;
    } else {
        tmp_progress = this->progress - 1;
        dir = 0;
    }

    if (type <= 2) {
        if (dir == 0) {
            if (type == 0 || type == 1) {
                return {Pos(tmp_progress, 0)};
            } else {
                return {Pos(tmp_progress - 1, 1)};
            }
        } else {
            if (type == 0 || type == 1) {
                return {Pos(this->f.size - 1, 2 + tmp_progress)};
            } else {
                return {Pos(this->f.size - 2, 2 + tmp_progress - 1)};
            }
        }
    } else if (type <= 7) {
        type -= 3;

        if (type == 0 || type == 2) {
            res = {Pos(tmp_progress, 0), Pos(tmp_progress + 1, 0)};
        } else if (type == 1) {
            res = {Pos(tmp_progress, 0), Pos(tmp_progress, 1)};
        } else if (type == 3 || type == 4) {
            res = {Pos(tmp_progress - 1, 1), Pos(tmp_progress + 1, 0)};
        }

        if (dir == 1) {
            rep (i, res.size()) {
                res[i] = Pos(this->f.size - 1 - res[i].y, res[i].x + 2);
            }
        }
    } else {
        type -= 8;
        int x = this->last_pair_x - 1;
        int y = tmp_progress - x;
        if (type == 3 || type == 5) y--;
        
        Pos tmp = Pos(x, y);
        if (type == 1 || type == 4 || type == 5) tmp.x++;
        if (dir) {
            tmp = getRotatePos(tmp, Ope(0, 0, this->f.size));
            tmp.y += 2;
        }
        return {tmp};
    }
    return res;
}

v_pos State::getLastPos(int type) {
    v_pos res;
    int tmp_progress, dir;
    if (this->progress >= this->f.size + 1) {
        tmp_progress = this->progress - (this->f.size + 1);
        dir = 1;
    } else {
        tmp_progress = this->progress - 1;
        dir = 0;
    }

    if (type <= 2) {
        v_pos base_pos = this->getBasePos(type);
        if (dir == 0) {
            if (type == 0) {
                return {base_pos[0] + Pos(0, 1)};
            } else {
                return {base_pos[0] + Pos(1, 0)};
            }
        } else {
            if (type == 0) {
                return {base_pos[0] + Pos(-1, 0)};
            } else {
                return {base_pos[0] + Pos(0, 1)};
            }
        }
    } else if (type <= 7) {
        type -= 3;

        if (type == 0) {
            res = {Pos(tmp_progress, 1), Pos(tmp_progress + 1, 1)};
        } else if (type == 1) {
            res = {Pos(tmp_progress + 1, 0), Pos(tmp_progress + 1, 1)};
        } else if (type == 2) {
            res = {Pos(tmp_progress, 1), Pos(tmp_progress + 2, 0)};
        } else if (type == 3) {
            res = {Pos(tmp_progress, 1), Pos(tmp_progress + 1, 1)};
        } else if (type == 4) {
            res = {Pos(tmp_progress, 1), Pos(tmp_progress + 2, 0)};
        }

        if (dir == 1) {
            rep (i, res.size()) {
                res[i] = Pos(this->f.size - 1 - res[i].y, res[i].x + 2);
            }
        }
    } else {
        // 実際使わないので実装していません
    }
    return res;
}

v_ope State::getToHorizonOpe() {
    int tmp_progress, dir;
    if (this->progress > this->f.size + 1) {  // =はいらない？
        tmp_progress = this->progress - (this->f.size + 1);
        dir = 1;
    } else {
        tmp_progress = this->progress - 1;
        dir = 0;
    }

    int size = tmp_progress - this->last_pair_x + 1;
    // if (size < 0) {
    //     cout << size << " " << tmp_progress << " " << this->last_pair_x << endl;
    //     cout << this->progress << endl;
    //     exit(1);
    // }
    v_ope res;
    if (dir) {
        if (this->f.getEnt(this->last_pair_x, this->f.size - 1).val == this->f.getEnt(this->last_pair_x + 1, this->f.size - 1).val) {
            res.push_back(Ope(this->f.size - 2, this->last_pair_x, 2));
        }
        res.push_back(Ope(this->f.size - size, this->last_pair_x + 1, size));
    } else {
        if (this->f.getEnt(0, this->last_pair_x - 2).val == this->f.getEnt(0, this->last_pair_x - 1).val) {
            res.push_back(Ope(this->last_pair_x - 2, 0, 2));
        }
        res.push_back(Ope(this->last_pair_x - 1, 0, size));
    }
    return res;
}