#include <bits/stdc++.h>
using namespace std;
#define rep(i, n) for (int i = 0; i < n; i++)

vector<vector<int>> g;
vector<vector<vector<int>>> ans_list;
int H, W;
int ans = 0;

char *file_name = NULL;
FILE *fp;

void dfs(int cnt);

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            i++;
            file_name = argv[i];
        }
    }

    cin >> H >> W;
    g = vector<vector<int>>(H, vector<int>(W, -1));
    dfs(0);
    cout << endl << ans << endl;

    if (file_name != NULL) {
        fp = fopen(file_name, "w");
        fprintf(fp, "%d\n", ans);
        for (vector<vector<int>> &grid : ans_list) {
            rep(i, H) rep(j, W) fprintf(fp, "%d%c", grid[i][j], " \n"[j == W - 1]);
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
    return 0;
}

void dfs(int cnt) {
    if (cnt == H * W / 2) {
        ans++;
        ans_list.push_back(g);
        cout << "." << flush;
        return;
    }
    rep(i, H) rep(j, W) {
        if (g[i][j] != -1) continue;
        if (i + 1 < H && g[i + 1][j] == -1) {
            g[i][j] = g[i + 1][j] = cnt;
            dfs(cnt + 1);
            g[i][j] = g[i + 1][j] = -1;
        }

        if (j + 1 < W && g[i][j + 1] == -1) {
            g[i][j] = g[i][j + 1] = cnt;
            dfs(cnt + 1);
            g[i][j] = g[i][j + 1] = -1;
        }
        return;
    }
}