#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <array>
#include <vector>
#include <stdint.h>
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

// HTTP通信とJSON処理のためのライブラリ
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

typedef std::vector<uint16_t> RawField;

// function define

struct Ope {
  std::array<uint8_t, 3> data;
  Ope();
  Ope(uint8_t x, uint8_t y, uint8_t n);
  Ope(int x, int y, int n);
  bool operator < (const Ope &other);
  uint8_t x() const;
  uint8_t y() const;
  uint8_t n() const;
};

void getProblem(RawField& field, uint32_t& fsize);
void submission(std::vector<Ope> result);
RawField createRandomField(uint32_t fsize, size_t seed=0);

//Inline implementation
inline Ope::Ope() : data({0, 0, 0}) {}
inline Ope::Ope(uint8_t x, uint8_t y, uint8_t n) : data({x, y, n}) {}
inline Ope::Ope(int x, int y, int n) : data({static_cast<uint8_t>(x), static_cast<uint8_t>(y), static_cast<uint8_t>(n)}) {}

inline bool Ope::operator<(const Ope &other) {
  return this->data[0] < other.data[0]
        || (this->data[0] == other.data[0] && this->data[1] < other.data[1])
        || (this->data[0] == other.data[0] && this->data[1] == other.data[1] && this->data[2] < other.data[2]);
}

inline uint8_t Ope::x() const { return this->data[0]; }
inline uint8_t Ope::y() const { return this->data[1]; }
inline uint8_t Ope::n() const { return this->data[2]; }

inline RawField createRandomField(uint32_t fsize, size_t seed) {
  const uint16_t n = fsize * fsize / 2;
  RawField result;
  result.reserve(n * 2);

  for(uint16_t i = 0; i < n; ++i) {
    result.push_back(i);
    result.push_back(i);
  }

  if(seed == 0){
    static std::random_device rd;
    seed = rd();
  }

  static std::mt19937 gen(seed);
  std::shuffle(result.begin(), result.end(), gen);
  return result;
}

// cURLのレスポンスを受け取るためのコールバック関数 (submission用)
inline size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
  userp->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// --- API設定 ---
// TODO: 実際の環境に合わせて変更してください
const std::string API_BASE_URL = "http://localhost:3000"; 
const std::string YOUR_TOKEN = "player1";

// const std::string API_BASE_URL = "http://192.168.123.2:3000";
// const std::string YOUR_TOKEN = "YSha9PPxJ4Yn";

// サーバーから問題を取得する関数
inline void getProblem(RawField& field, uint32_t& fsize) {
  bool is_connection = false;
  std::cout << "試合情報の取得を開始します..." << std::endl;
  std::cout << "APIサーバー: " << API_BASE_URL << std::endl;

  while (!is_connection) {
    cpr::Response response =
        cpr::Get(cpr::Url{API_BASE_URL + "/"},
                 cpr::Header{{"Procon-Token", YOUR_TOKEN}});

    if (response.status_code == 200) {
      std::cout << "サーバーから正常なレスポンスを受信しました。" << std::endl;
      try {
        json data = json::parse(response.text);

        if (data.contains("problem") && data["problem"].contains("field")) {
          auto field_data = data["problem"]["field"];
          
          fsize = field_data["size"];
          std::cout << "フィールドサイズ: " << fsize << "x" << fsize << std::endl;

          auto entities = field_data["entities"].get<std::vector<std::vector<int>>>();
          
          field.clear();
          field.reserve(fsize * fsize);
          for (const auto &row : entities) {
            for (int cell : row) {
              field.push_back(static_cast<uint16_t>(cell));
            }
          }
          is_connection = true; // 取得成功したらループを抜ける
        } else if (data.contains("startsAt")) {
          std::cout << "試合が始まるのを待っています..." << std::endl;
        } else {
          std::cerr << "エラー: 受信したJSONの形式が正しくありません。" << std::endl;
          std::cerr << "受信データ: " << data.dump(4) << std::endl;
        }
      } catch (json::parse_error &e) {
        std::cerr << "JSONのパースに失敗しました: " << e.what() << std::endl;
      }
    } else {
      std::cerr << "エラー: 受信時にエラーが起きました。ステータスコード: " << response.status_code << std::endl;
    }
    
    if(!is_connection){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

// サーバーに回答を送信する関数
inline void submission(std::vector<Ope> result) {
  std::cout << "result size: " << result.size() << std::endl;

  json answer_data;
  answer_data["ops"] = json::array();
  for (const auto& op : result) {
    answer_data["ops"].push_back({{"x", op.x()}, {"y", op.y()}, {"n", op.n()}});
  }

  std::string json_payload = answer_data.dump();

  /* CURL *curl; */
  /* CURLcode res; */
  /* std::string readBuffer; */

  /* curl_global_init(CURL_GLOBAL_DEFAULT); */
  /* curl = curl_easy_init(); */
  /* if (curl) { */
  /*   curl_easy_setopt(curl, CURLOPT_URL, (API_BASE_URL).c_str()); // APIエンドポイント */
  /*   curl_easy_setopt(curl, CURLOPT_POST, 1L); */
  /*   curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str()); */

  /*   struct curl_slist *headers = NULL; */
  /*   headers = curl_slist_append(headers, "Content-Type: application/json"); */
  /*   headers = curl_slist_append(headers, ("Procon-Token: " + YOUR_TOKEN).c_str()); */
  /*   curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); */

  /*   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); */
  /*   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); */

  /*   res = curl_easy_perform(curl); */
  /*   if (res != CURLE_OK) { */
  /*     std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl; */
  /*   } else { */
  /*     long response_code; */
  /*     curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code); */
  /*     std::cout << "Response Code: " << response_code << std::endl; */
  /*     std::cout << "Response: " << readBuffer << std::endl; */
  /*   } */

  /*   curl_slist_free_all(headers); */
  /*   curl_easy_cleanup(curl); */
  /* } */
  /* curl_global_cleanup(); */

  // 回答をファイルにも保存
  std::string filename = "answer.json";
  std::ofstream ofs(filename);
  if (ofs) {
    ofs << json_payload;
    std::cout << "回答をファイル '" << filename << "' に保存しました。" << std::endl;
  }
}

#endif
