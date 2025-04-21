let is_CSV_check_box = document.getElementById("isCSV");
let is_include_N_check_box = document.getElementById("isIncludeN");
let input_text = document.getElementById("input");
let output_text = document.getElementById("output");
let is_loop_check_box = document.getElementById("isLoop");
let is_smooth_check_box = document.getElementById("isSmooth");
let play_button = document.getElementById("play");
let speed_range = document.getElementById("speed");
let turn_number = document.getElementById("turn");
let t_bar_range = document.getElementById("t_bar");
let result_div = document.getElementById("result");

let field_list = [];
let ent_pos_list = [];
let ope_list = [];
let ok_pair_cnt = [];
let turn_cnt = [];

let now_turn = 0;

let last_animation_time;

is_CSV_check_box.addEventListener("click", (event) => {
    play_button.value = "▶";
    update_data();
});

is_include_N_check_box.addEventListener("click", (event) => {
    play_button.value = "▶";
    update_data();
});

input_text.addEventListener("input", (event) => {
    play_button.value = "▶";
    update_data();
});

output_text.addEventListener("input", (event) => {
    play_button.value = "▶";
    update_data();
});

play_button.addEventListener("click", (event) => {
    console.log(event);
    if (play_button.value == "▶") {
        startAutoPlay();
    } else {
        play_button.value = "▶";
    }
});

turn_number.addEventListener("input", (event) => {
    play_button.value = "▶";
    setTrun(turn_number.value);
    // output_field_as_table(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, result_div);
    output_field_rotate(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, 0, result_div);
});

t_bar_range.addEventListener("input", (event) => {
    play_button.value = "▶";
    setTrun(t_bar_range.value);
    //output_field_as_table(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, result_div);
    output_field_rotate(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, 0, result_div);
});

function update_data() {
    let result = solve(input_text.value, output_text.value, is_CSV_check_box.checked, is_include_N_check_box.checked, result_div);
    field_list = result[1];
    ent_pos = result[2];
    ope_list = result[3];
    ok_pair_cnt = result[4];
    now_turn = turn_cnt = result[5];

    if (result[0]) {
        t_bar_range.value = t_bar_range.max = turn_number.value = turn_number.max = now_turn;
        //output_field_as_table(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, result_div);
        output_field_rotate(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, 0, result_div);
    } else {
        t_bar_range.max = t_bar_range.value = turn_number.max = turn_number.value = 0;
        result_div.innerHTML = "<font color=\"red\">Input is not available.</font>";
    }
}

function output_field_as_table(field, ent_pos, ope, ok_pair_cnt, turn_cnt, result_div) {
    let N = field.length;
    let tmp_result = "<table>";
    for (let i = 0; i < N; i++) {
        tmp_result += "<tr>";
        for (let j = 0; j < N; j++) {
            let val = field[i][j].val
            let pair_pos = ent_pos[val][(field[i][j].num + 1) % 2];
            let cell_class = "result_cell";
            if (isPair(ent_pos[val][0], ent_pos[val][1])) {
                cell_class += " ok_pair";
            }
            if (ope != null && i >= ope.y && i < ope.y + ope.n && j >= ope.x && j < ope.x + ope.n) {
                cell_class += " operated";
            }
            tmp_result += `<td class="${cell_class}" title="val : ${val}\n(${j}, ${i})\nPair : (${pair_pos.x}, ${pair_pos.y})">${field[i][j].val}</td>`;
        }
        tmp_result += "</tr>";
    }
    tmp_result += "</table>";
    result_div.innerHTML = `Pair cnt : ${ok_pair_cnt}<br>Ope cnt : ${turn_cnt}<br>${tmp_result}`;

    result_cells = result_div.querySelectorAll(".result_cell");
    result_cells.forEach((cell) => {
        cell.addEventListener("mouseenter", (event) => {
            result_cells.forEach((cell2) => {
                if (cell.textContent == cell2.textContent) {
                    cell2.classList.add("bold");
                }
            })
        });

        cell.addEventListener("mouseleave", (event) => {
            result_cells.forEach((cell2) => {
                if (cell.textContent == cell2.textContent) {
                    cell2.classList.remove("bold");
                }
            })
        });
    })
}

function output_field_rotate(field, ent_pos, ope, ok_pair_cnt, turn_cnt, rotate, result_div) {
    let N = field.length;
    let grid_div = document.createElement("div");
    let not_rotate_div = document.createElement("div");
    let rotate_div = document.createElement("div");
    grid_div.classList.add("grid");
    not_rotate_div.classList.add("not_rotate");
    rotate_div.classList.add("rotate");

    for (let i = 0; i < N; i++) {
        for (let j = 0; j < N; j++) {
            let cell_div = document.createElement("div");
            let val = field[i][j].val;
            let pair_pos = ent_pos[val][(field[i][j].num + 1) % 2];
            cell_div.title = `val : ${val}\n(${j}, ${i})\nPair : (${pair_pos.x}, ${pair_pos.y})`;
            cell_div.classList.add("result_cell");
            cell_div.textContent = field[i][j].val;
            if (isPair(ent_pos[val][0], ent_pos[val][1])) {
                cell_div.classList.add("ok_pair");
            }
            if (ope != null && i >= ope.y && i < ope.y + ope.n && j >= ope.x && j < ope.x + ope.n) {
                cell_div.classList.add("operated");
                cell_div.style.cssText = `
                top : ${(i - ope.y) * 30}px;
                left : ${(j - ope.x) * 30}px;
                /*transform : rotate(${-rotate * 90}deg);*/
                `;
                rotate_div.appendChild(cell_div);
            } else {
                cell_div.style.cssText = `
                top : ${i * 30}px;
                left : ${j * 30}px;
                `;
                not_rotate_div.appendChild(cell_div);
            }
        }
    }

    grid_div.style.cssText = `
    width: ${N * 30}px;
    height: ${N * 30}px;
    `;
    if (ope != null){
        rotate_div.style.cssText = `
        top : ${ope.y * 30}px;
        left : ${ope.x * 30}px;
        width : ${ope.n * 30}px;
        height : ${ope.n * 30}px;
        transform : rotate(${rotate * 90}deg);
        `;
    }

    grid_div.appendChild(not_rotate_div);
    grid_div.appendChild(rotate_div);
    result_div.innerHTML = `<p>Pair cnt : ${ok_pair_cnt}<br>Ope cnt : ${turn_cnt}</p>`;
    result_div.appendChild(grid_div);

    result_cells = grid_div.querySelectorAll(".result_cell");
    console.log(result_cells);
    result_cells.forEach((cell) => {
        cell.addEventListener("mouseenter", (event) => {
            result_cells.forEach((cell2) => {
                if (cell.textContent == cell2.textContent) {
                    cell2.classList.add("bold");
                }
            });
        });

        cell.addEventListener("mouseleave", (event) => {
            result_cells.forEach((cell2) => {
                if (cell.textContent == cell2.textContent) {
                    cell2.classList.remove("bold");
                }
            });
        });
    });
}

function setTrun(turn) {
    now_turn = turn;
    turn_number.value = turn;
    t_bar_range.value = turn;
}

function startAutoPlay() {
    if (turn_cnt == 0 || (now_turn == turn_cnt && !is_loop_check_box.checked)) {
        return false;
    }
    last_animation_time = Date.now();
    // output_field_as_table(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, result_div);
    output_field_rotate(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, 0, result_div);
    play_button.value = "■";
    return true;
}

function autoPlay() {
    if (play_button.value == "■") {
        let now_time = Date.now();
        let delta_time = 2000 / pow(1.04713, speed_range.value);
        let sub_time = now_time - last_animation_time
        if (sub_time >= delta_time) {
            setTrun((Number(now_turn) + 1) % (turn_cnt + 1));
            if (!is_smooth_check_box.checked){
                output_field_rotate(field_list[now_turn], ent_pos[now_turn], now_turn == 0 ? null : ope_list[now_turn - 1], ok_pair_cnt, turn_cnt, 0, result_div);
            }
            if (now_turn == turn_cnt && !is_loop_check_box.checked) {
                play_button.value = "▶";
            }
            last_animation_time = now_time;
        }
        if (is_smooth_check_box.checked){
            output_field_rotate(field_list[now_turn], ent_pos[now_turn], now_turn == turn_cnt ? null : ope_list[now_turn], ok_pair_cnt, turn_cnt, sub_time / delta_time, result_div);
        }
    }   
    requestAnimationFrame(autoPlay);
}
autoPlay();

function setSample(){
    input_text.value = "0 1 2 3\n4 5 6 7\n0 1 2 3\n4 5 6 7";
    output_text.value = "0 1 2\n0 2 2\n1 1 2\n1 2 2\n2 1 2\n2 0 2\n0 2 2\n1 2 2\n2 1 2\n1 2 2\n1 2 2\n1 2 2\n2 2 2\n0 2 2\n0 2 2\n1 2 2";
    update_data(); 
}
setSample();