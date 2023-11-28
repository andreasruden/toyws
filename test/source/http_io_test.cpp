#include <catch2/catch_test_macros.hpp>
#include <string>

#include "toyws/http_request.hpp"

// Get request to /
auto getRequest = std::string(
    "GET / HTTP/1.1\r\n"
    "Host: 127.0.0.1:5000\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:120.0) "
    "Gecko/20100101 Firefox/120.0\r\n"
    "Accept: "
    "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/"
    "webp,*/*;q=0.8\r\n"
    "Accept-Language: en-US,sv;q=0.7,en;q=0.3\r\n"
    "Accept-Encoding: gzip, deflate, br\r\n"
    "Connection: keep-alive\r\n"
    "Upgrade-Insecure-Requests: 1\r\n"
    "Sec-Fetch-Dest: document\r\n"
    "Sec-Fetch-Mode: navigate\r\n"
    "Sec-Fetch-Site: none\r\n"
    "Sec-Fetch-User: ?1\r\n\r\n");

// POST request to /some/form for a form with two text inputs: fname and lname
auto postFormRequest = std::string(
    "POST /some/form HTTP/1.1\r\n"
    "Host: 127.0.0.1:5000\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:120.0) "
    "Gecko/20100101 Firefox/120.0\r\n"
    "Accept: "
    "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/"
    "webp,*/*;q=0.8\r\n"
    "Accept-Language: en-US,sv;q=0.7,en;q=0.3\r\n"
    "Accept-Encoding: gzip, deflate, br\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 25\r\n"
    "Origin: null\r\n"
    "Connection: keep-alive\r\n"
    "Upgrade-Insecure-Requests: 1\r\n"
    "Sec-Fetch-Dest: document\r\n"
    "Sec-Fetch-Mode: navigate\r\n"
    "Sec-Fetch-Site: cross-site\r\n"
    "Sec-Fetch-User: ?1\r\n"
    "\r\n"
    "fname=Smith&lname=Johnson");

TEST_CASE("Basic Get Request", "[library]") {
  toyws::HttpRequest req;
  REQUIRE(req.Read(getRequest.data(), getRequest.size()) == true);

  REQUIRE(req.Method() == toyws::HttpMethod::GET);

  REQUIRE(req.Headers().at("Host") == "127.0.0.1:5000");
  REQUIRE(req.Headers().at("Accept-Language") == "en-US,sv;q=0.7,en;q=0.3");
  REQUIRE(req.Headers().at("Accept-Encoding") == "gzip, deflate, br");

  REQUIRE(req.Body().empty());
}

TEST_CASE("Basic Post Request", "[library]") {
  toyws::HttpRequest req;
  REQUIRE(req.Read(postFormRequest.data(), postFormRequest.size()) == true);

  REQUIRE(req.Method() == toyws::HttpMethod::POST);

  REQUIRE(req.Headers().at("Host") == "127.0.0.1:5000");
  REQUIRE(req.Headers().at("Content-Length") == "25");

  REQUIRE(req.Body().size() == 25);
  REQUIRE(req.Body() == "fname=Smith&lname=Johnson");
}
