#include "../inc/all.hpp"
#include <chrono>
#include <cpr/cpr.h>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

// nlohmann::jsonの名前空間を省略して使えるようにする
using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp) {
  userp->append((char *)contents, size * nmemb);
  return size * nmemb;
}

/// @brief APIから試合情報を取得し、フィールド情報を表示する関数
/// @param api_base_url APIサーバーのベースURL
/// @param your_token 認証トークン
/// @return 試合情報の取得に成功した場合はtrue、それ以外はfalse
json get_match_info(const std::string &api_base_url,
                    const std::string &your_token) {
  while (true) {
    cpr::Response response =
        cpr::Get(cpr::Url{api_base_url + "/"},
                 cpr::Header{{"Procon-Token", your_token}});

    if (response.status_code == 200) {
      std::cout << "サーバーから正常なレスポンスを受信しました。" << std::endl;
      try {
        json data = json::parse(response.text);
        if (data.contains("problem") && data["problem"].contains("field")) {
          // フィールド情報を表示 (デバッグ用)
          std::cout << "フィールドサイズ: " << data["problem"]["field"]["size"]
                    << std::endl;
          return data; // 問題データを返す
        } else if (data.contains("startsAt")) {
          std::cout << "試合が始まるのを待っています..." << std::endl;
        } else {
          std::cerr << "エラー: 受信JSONの形式が不正です。" << std::endl;
        }
      } catch (json::parse_error &e) {
        std::cerr << "JSONのパースに失敗しました: " << e.what() << std::endl;
      }
    } else {
      std::cerr << "エラー: ステータスコード " << response.status_code
                << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

// post_answer関数は変更なしのため省略...
// (ご自身のコードにはこの関数も含めてください)

// ----------------------------------------------------------------
// 新しい loadProblem 関数
// ----------------------------------------------------------------

/// @brief APIから問題データを取得し、Stateオブジェクトを構築して返す
/// @param api_base_url APIサーバーのベースURL
/// @param your_token 認証トークン
/// @return 問題データが格納されたStateオブジェクト
State loadProblem(const std::string &api_base_url,
                  const std::string &your_token) {
  // APIから問題情報をJSON形式で取得
  json problem_data = get_match_info(api_base_url, your_token);
  cout << "get_match_info完了" << endl;

  if (!problem_data.contains("problem")) {
    std::cerr << "エラー: JSONデータに 'problem' キーが見つかりません。"
              << std::endl;
    exit(1);
  }

  auto field_data = problem_data["problem"]["field"];
  int N = field_data["size"].get<int>();
  auto entities = field_data["entities"].get<std::vector<std::vector<int>>>();

  // Stateオブジェクトを初期化
  State res;
  res.f.size = N;
  res.f.ent_mem = new Ent[N * N];
  res.f.pos_mem =
      new Pos[N * N]; // 各値が2つずつあるのでN*N/2 * 2 = N*N個の領域が必要

  // 各値が何回出現したかをカウントする配列
  int max_val = (N * N) / 2;
  int *cnt = new int[max_val](); // ()でゼロ初期化

  // 取得したエンティティ情報をStateオブジェクトに格納
  rep(i, N) {   // i: 行 (y座標)
    rep(j, N) { // j: 列 (x座標)
      int val = entities[i][j];

      if (val >= max_val) {
        std::cerr << "エラー: エンティティの値 " << val << " が想定範囲外です。"
                  << std::endl;
        // 必要に応じてエラー処理
      }

      // 盤面情報を設定
      res.f.getEnt(i, j).val = val;
      res.f.getEnt(i, j).num = cnt[val];

      // 逆引きテーブルを設定
      res.f.getEntPos(val, cnt[val]) = (Pos){(uint8_t)j, (uint8_t)i};

      cnt[val]++;
    }
  }

  // 動的に確保したメモリを解放
  delete[] cnt;

  std::cout << "問題の読み込みが完了しました。" << std::endl;
  return res;
}

void post_answer(const std::string &api_base_url, const std::string &your_token,
                 std::string &answer) {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    // APIエンドポイントを設定
    curl_easy_setopt(curl, CURLOPT_URL, api_base_url.c_str());

    // HTTPメソッドをPOSTに設定
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // リクエストボディを設定
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, answer.c_str());

    // HTTPヘッダーを設定
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers =
        curl_slist_append(headers, ("Procon-Token: " + your_token).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // レスポンスデータを取得するためのコールバック関数を設定
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // リクエストを実行
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
                << std::endl;
    } else {
      std::cout << "Response: " << readBuffer << std::endl;
    }

    // 後処理
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();

  std::string filename = "answer.json";
  std::ofstream ofs(filename);
  if (ofs) {
    ofs << answer;
    std::cout << "回答をファイル '" << filename << "' に保存しました。"
              << std::endl;
  } else {
    std::cerr << "エラー: ファイル '" << filename << "' を開けませんでした。"
              << std::endl;
  }
}