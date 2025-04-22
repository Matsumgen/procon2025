class Pos {
    constructor(x, y){
        this.x = x;
        this.y = y;
    }
}

class Ent {
    constructor(val, num){
        this.val = val;
        this.num = num;
    }
}

class Ope {
    constructor(x, y, n){
        this.x = x;
        this.y = y;
        this.n = n;
    }
}

function solve(input_text, output_text, is_CSV, is_include_N, result_div){
    let field_list = [];
    let ent_pos_list = [];
    let ope_list = [];

    let set_field_result = setField(input_text, is_CSV, is_include_N);
    if (!set_field_result[2]){
        return [false, [], [], [], 0, 0];
    }

    let cnt = 0;
    field_list.push(set_field_result[0]);
    ent_pos_list.push(set_field_result[1]);
    let str_ope_list = output_text.split("\n");
    try {
        for (let i = 0; i < str_ope_list.length; i++){
            if (str_ope_list[i] == "") continue;
            // let tmp = str_ope_list[i].split(" ");
            let tmp = str_ope_list[i].split(/ +/).filter((s) => s !== "");
            if (tmp.length != 3) return [false, [], [], [], 0, 0]; 
            ope_list.push(new Ope(Number(tmp[0]), Number(tmp[1]), Number(tmp[2])));
            let next_state = rotate(Number(tmp[0]), Number(tmp[1]), Number(tmp[2]), field_list[cnt], ent_pos_list[cnt]);
            if (!next_state[0]) return [false, [], [], [], 0, 0]; 
            field_list.push(next_state[1]);
            ent_pos_list.push(next_state[2]);
            cnt++;
        }
    }catch (error){
        return [false, [], [], [], 0, 0];
    }

    let pair_cnt = 0;
    for (let i = 0; i < ent_pos_list[cnt].length; i++){
        pair_cnt += isPair(ent_pos_list[cnt][i][0], ent_pos_list[cnt][i][1]);
    }
    return [true, field_list, ent_pos_list, ope_list, pair_cnt, cnt];
}

function setField(input_text, is_CSV, is_include_N){
    let lines = input_text.split("\n");
    let idx = 0;
    let tmp_field = [];
    let N;
    let field = [];
    let ent_pos = [];
    if (is_include_N){
        N = Number(lines[idx]);
        idx++;
    }
    while (idx < lines.length){
        if (lines[idx] == "") {
            idx++;
            continue;
        }
        // let values = lines[idx].split(is_CSV ? "," : " ");
        let values = lines[idx].split(is_CSV ? /,+/ : / +/).filter((s) => s !== "");
        tmp_field.push(values);
        idx++;
    }

    if (tmp_field.length < 4 || tmp_field.length % 2 || (is_include_N && tmp_field.length != N)) {
        return [[], [], false];
    }
    N = tmp_field.length;
    let val_cnt = N * N / 2;
    try {
        field = new Array(N);
        ent_pos = new Array(val_cnt);
        for (let i = 0; i < N; i++) field[i] = new Array(N);
        for (let i = 0; i < val_cnt; i++) ent_pos[i] = [];
        for (let i = 0; i < N; i++){
            for (let j = 0; j < N; j++){
                let val = new Number(tmp_field[i][j]);
                field[i][j] = new Ent(val, ent_pos[val].length);
                ent_pos[val].push(new Pos(j, i));
            }
        }
    }catch (error){
        console.log(error);
        return [[], [], false];
    }

    for (let i = 0; i < val_cnt; i++){
        if (ent_pos[i].length != 2){
            return [[], [], false];
        }
    }
    return [field, ent_pos, true];
}

function getFieldCopy(field){
    let N = field.length;
    let copy_field = new Array(N);
    for (let i = 0; i < N; i++) copy_field[i] = new Array(N);
    for (let i = 0; i < N; i++) for (let j = 0; j < N; j++){
        copy_field[i][j] = new Ent(field[i][j].val, field[i][j].num);
    }
    return copy_field;
}

function getEntPosCopy(ent_pos){
    let pair_cnt = ent_pos.length;
    let copy_ent_pos = new Array(pair_cnt);
    for (let i = 0; i < pair_cnt; i++) copy_ent_pos[i] = new Array(2);
    for (let i = 0; i < pair_cnt; i++) for (let j = 0; j < 2; j++) {
        copy_ent_pos[i][j] = new Pos(ent_pos[i][j].x, ent_pos[i][j].y);
    }
    return copy_ent_pos;
}

function rotate(x, y, n, field, ent_pos){
    let next_field = getFieldCopy(field);
    let next_ent_pos = getEntPosCopy(ent_pos);
    try {
        for (let i = 0; i < n; i++) for (let j = 0; j < n; j++) {
            next_field[y + i][x + j] = field[y + n - j - 1][x + i];
            next_ent_pos[next_field[y + i][x + j].val][next_field[y + i][x + j].num] = new Pos(x + j, y + i);
        }
    } catch (error){
        return [false, [], []];
    }
    return [true, next_field, next_ent_pos];
}

function abs(x){
    return x >= 0 ? x : -x;
}

function pow(a, n){
    let res = 1;
    let tmp = a;
    for (let i = 0; i < 10; i++){
        if ((n >> i) & 1){
            res *= tmp;
        }
        tmp *= tmp;
    }
    return res;
}
function isPair(ent1, ent2){
    let tmp = new Pos(abs(ent2.x - ent1.x), abs(ent2.y - ent1.y));
    return ((tmp.x == 0 && tmp.y == 1) || (tmp.x ==1 && tmp.y == 0));
}