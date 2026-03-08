#include <curl/curl.h>
#include <fstream> // ファイル読み込みのために追加
#include <iostream>
#include <sstream> // ファイル内容を文字列に変換するために追加
#include <string>

// cURLコールバック関数
size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp) {
  userp->append((char *)contents, size * nmemb);
  return size * nmemb;
}

int main() {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;

  // APIエンドポイントとトークン
  const std::string api_url =
      "http://localhost:3000"; // 実際のAPIサーバーのURLに置き換えてください
  const std::string procon_token =
      "player1"; // 事前配布されたトークンに置き換えてください

  // --- answer.jsonファイルから回答データを読み込む ---
  std::ifstream ifs("answer.json"); // 回答ファイルのパスを書き込む
  if (!ifs.is_open()) {
    std::cerr << "エラー: answer.jsonファイルを開けませんでした。" << std::endl;
    return 1; // エラー終了
  }
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  std::string json_payload = buffer.str();
  // ------------------------------------------------

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    // APIエンドポイントを設定
    curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());

    // HTTPメソッドをPOSTに設定
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // リクエストボディを設定 (ファイルから読み込んだ内容)
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
      std::cout << "Response: " << readBuffer << std::endl;
    }

    // 後処理
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();

  return 0;
}