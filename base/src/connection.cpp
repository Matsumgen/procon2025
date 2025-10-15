#include <chrono>
#include <cpr/cpr.h> // For making HTTP requests
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp> // For parsing JSON
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

  bool is_connection = false;
  while (!is_connection) {
    // --- APIリクエストの送信 ---
    // API仕様書によると、試合情報取得APIのエンドポイントは "/"
    // 認証トークンは "Procon-Token" ヘッダーに設定
    cpr::Response response =
        cpr::Get(cpr::Url{api_base_url + "/"},
                 cpr::Header{{"Procon-Token", your_token}});

    // --- レスポンスの処理 ---
    // ステータスコードが200 (正常) の場合
    if (response.status_code == 200) {
      std::cout << "サーバーから正常なレスポンスを受信しました。" << std::endl;

      try {
        // レスポンスのテキストをJSONとしてパースする
        json data = json::parse(response.text);

        // 仕様書に基づき、"problem" -> "field" の階層をたどる
        if (data.contains("problem") && data["problem"].contains("field")) {
          auto field_data = data["problem"]["field"];

          // フィールドサイズを取得
          int size = field_data["size"];
          std::cout << "フィールドサイズ: " << size << "x" << size << std::endl;

          // エンティティ情報（二次元配列）を取得
          auto entities =
              field_data["entities"].get<std::vector<std::vector<int>>>();

          // 取得したフィールド情報を表示
          std::cout << "フィールド情報:" << std::endl;
          for (const auto &row : entities) {
            for (int cell : row) {
              std::cout << cell << "\t";
            }
            std::cout << std::endl;
          }
          return data;
        } else if (data.contains("startsAt")) {
          std::cout << "試合が始まるのを待っています" << std::endl;
        } else {
          std::cerr
              << "エラー: 受信したJSONの形式が正しくありません ('problem' "
                 "'field' ' キーが見つかりません)。"
              << std::endl;
          std::cerr << "受信データ: " << data.dump(4)
                    << std::endl; // 整形して表示
        }
      } catch (json::parse_error &e) {
        std::cerr << "JSONのパースに失敗しました: " << e.what() << std::endl;
      }
    }
    // エラーの場合
    else {
      std::cerr << "エラー: 受信時にエラーが起きました" << std::endl;
      std::cerr << "ステータスコード: " << response.status_code << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return false;
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

int main() {
  // --- 設定項目 ---
  // TODO: 以下の2つの値をあなたの環境に合わせて書き換えてください
  std::string api_base_url =
      "http://localhost:3000";        // 例: "http://localhost:8080"
  std::string your_token = "player1"; // 例: "my-secret-token"

  std::cout << "試合情報の取得を開始します..." << std::endl;
  std::cout << "APIサーバー: " << api_base_url << std::endl;

  json problem_data;

  problem_data = get_match_info(api_base_url, your_token);

  json answer_data;
  answer_data["ops"] = json::array();
  answer_data["ops"].push_back({{"x", 0}, {"y", 0}, {"n", 2}});
  answer_data["ops"].push_back({{"x", 2}, {"y", 2}, {"n", 2}});

  std::string json_payload = answer_data.dump();

  post_answer(api_base_url, your_token, json_payload);
  return 0;
}