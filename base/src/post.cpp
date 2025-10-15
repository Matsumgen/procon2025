#include <curl/curl.h>
#include <fstream>   // ファイル読み込みのため
#include <iostream>
#include <sstream>   // 文字列ストリームのため
#include <string>
#include "nlohmann/json.hpp" // nlohmann/jsonライブラリをインクルード

// nlohmann::jsonを簡単のため別名で定義
using json = nlohmann::json;

// cURLのレスポンスを受け取るためのコールバック関数
size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp) {
  userp->append((char *)contents, size * nmemb);
  return size * nmemb;
}

int main() {
    std::cout << "Hellow" << std::endl;
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  // APIエンドポイントとトークン
  const std::string api_url = "http://localhost:3000"; // APIサーバーのURL
  const std::string procon_token = "player1";        // 自身のトークン

  // --- テキストファイルから回答データを読み込み、JSONを構築 ---
  std::ifstream ifs("C:/Users/sisim/Documents/procon/procon2025/Minami/fix_pos10/1th_ans.txt"); // 読み込むテキストファイルのパス
  if (!ifs.is_open()) {
    std::cerr << "エラー: answer.txtファイルを開けませんでした。" << std::endl;
    return 1; // エラー終了
  }

  json answer_data;
  answer_data["ops"] = json::array(); // "ops"キーに空の配列をセット

  std::string line;
  int x, y, n;

  // 1行目は操作回数なので読み飛ばす
  std::getline(ifs, line);

  // 2行目以降を1行ずつ読み込む
  while (std::getline(ifs, line)) {
    std::stringstream ss(line);
    // 1行から x, y, n の値を読み取る
    if (ss >> x >> y >> n) {
      // JSONオブジェクトを作成して配列に追加
      answer_data["ops"].push_back({{"x", x}, {"y", y}, {"n", n}});
    }
  }
  
  // JSONオブジェクトを文字列に変換
  std::string json_payload = answer_data.dump();
  // std::cout << "Generated JSON: " << json_payload << std::endl; // 送信するJSONを確認したい場合にコメントを外す
  // ----------------------------------------------------

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    // APIエンドポイントを設定
    curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());

    // HTTPメソッドをPOSTに設定
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // リクエストボディ（JSON文字列）を設定
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());

    // HTTPヘッダーを設定
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers =
        curl_slist_append(headers, ("Procon-Token: " + procon_token).c_str());
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
      // レスポンスコードを取得
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      std::cout << "Response Code: " << response_code << std::endl;
      std::cout << "Response Body: " << readBuffer << std::endl;
    }

    // 後処理
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();

  std::string filename = "answer.json";
  std::ofstream ofs(filename);
  if (ofs) {
    ofs << json_payload;
    std::cout << "回答ファイル'" << filename << "'に保存しました"
              << std::endl;
  } else {
    std::cerr << "エラー：'" << filename << "' を開けませんでした。"
              << std::endl;
  }

  return 0;
}