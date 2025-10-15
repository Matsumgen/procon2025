class Ope{
  int x;
  int y;
  int n;
  
  Ope (int x, int y, int n){
    this.x = x;
    this.y = y;
    this.n = n;
  }
  
  int[][] exe(int[][] field){
    int N = field.length;
    int[][] copy_field = getCopyArray2(field);
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) copy_field[i][j] = field[i][j];
    for (int i = 0; i < n; i++){
      for (int j = 0; j < n; j++){
        copy_field[y + i][x + j] = field[y + n - j - 1][x + i];
      }
    }
    return copy_field;
  }
}
