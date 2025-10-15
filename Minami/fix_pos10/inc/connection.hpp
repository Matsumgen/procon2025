#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "state.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp);

json get_match_info(const std::string &api_base_url,
                    const std::string &your_token);

State loadProblem(const std::string &api_base_url,
                  const std::string &your_token);

void post_answer(const std::string &api_base_url, const std::string &your_token,
                 std::string &answer);

#endif