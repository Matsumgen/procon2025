String load_file_path = "";  // ファイルのパスをここに指定(スペース区切りのテキストファイル)
int[][] field;
int N;
int rotate_x, rotate_y, rotate_n;
ArrayList<Ope> ope_list = new ArrayList<Ope>();
float rect_width, rect_height;

boolean is_drag = false;
int drag_bx = 0;
int drag_by = 0;
int first_field_bx = 50;
int first_field_by = 300;
int first_field_ex = 450;
int first_field_ey = 700;
int now_field_bx = 550;
int now_field_by = 300;
int now_field_ex = 950;
int now_field_ey = 700;

void setup() {
  size(1000, 1000);
  rectMode(CORNERS);
  field = load_file(load_file_path);
  rect_width = (now_field_ex - now_field_bx) / N;
  rect_height = (now_field_ey - now_field_by) / N;
}

void draw() {
  background(255);
    
  int[][] copy_field = getCopyArray2(field);
  for (Ope op : ope_list) {
    copy_field = op.exe(copy_field);
  }
  
  fill(0);
  textSize(35);
  textAlign(LEFT, CENTER);
  text("Before : ", first_field_bx, first_field_by - 30);
  text("Now : ", now_field_bx, now_field_by - 30);
  textAlign(RIGHT, CENTER);
  text("cnt : " + str(ope_list.size()), now_field_ex, now_field_by - 30);
  
  draw_field(field, first_field_bx, first_field_by, first_field_ex, first_field_ey);
  draw_field(copy_field, now_field_bx, now_field_by, now_field_ex, now_field_ey);
  
  if (is_drag) {
    fill(0, 150, 255, 100);
    noStroke();
    rect(drag_bx, drag_by, mouseX, mouseY);
  }
}

void mousePressed() {
  is_drag = true;
  drag_bx = mouseX;
  drag_by = mouseY;
}

void mouseReleased() {
  is_drag = false;
  int bx = (int)((drag_bx - now_field_bx) / rect_width);
  int by = (int)((drag_by - now_field_by) / rect_height);
  int ex = (int)((mouseX - now_field_bx) / rect_width);
  int ey = (int)((mouseY - now_field_by) / rect_height);
  println(bx, by, ex, ey);
  if (isContain(bx, 0, N - 1) && isContain(by, 0, N - 1) && isContain(ex, 0, N - 1) && isContain(ey, 0, N - 1)
    && abs(ex - bx) == abs(ey - by) && bx != ex) {
    ope_list.add(new Ope(min(bx, ex), min(by, ey), abs(ex - bx) + 1));
  }
}

void keyPressed() {
  if (keyCode == BACKSPACE && ope_list.size() != 0) {
    ope_list.remove(ope_list.size() - 1);
  }
}

int[][] load_file(String file_name) {
  String[] original_text = loadStrings(file_name);
  N = original_text.length;
  int[][] res = new int[N][N];
  for (int i = 0; i < N; i++) {
    String[] tmp_text = splitTokens(original_text[i]);
    for (int j = 0; j < N; j++) {
      res[i][j] = int(tmp_text[j]);
    }
  }
  return res;
}

int[][] getCopyArray2(int[][] array) {
  int H = array.length, W = array[0].length;
  int[][] copy_array = new int[H][W];
  for (int i = 0; i < H; i++) for (int j = 0; j < W; j++) copy_array[i][j] = array[i][j];
  return copy_array;
}

boolean isContain(int x, int min_val, int max_val) {
  return x >= min_val && x <= max_val;
}

void draw_field(int[][] field, int bx, int by, int ex, int ey) {
  stroke(0);
  for (int i = 0; i <= N; i++) {
    line(bx + rect_width * i, by, bx + rect_width  * i, ey);
  }
  for (int i = 0; i <= N; i++) {
    line(bx, by + rect_height * i, ex, by + rect_height  * i);
  }

  fill(0);
  textAlign(CENTER, CENTER);
  textSize(30);
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      text(field[i][j], bx + rect_width * (j + 0.5), by + rect_height * (i + 0.5));
    }
  }
}
