#include "connection.h"

#include <codecvt>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>

namespace httpsrvr {

namespace {

const std::string htmlPath =
    "C:/Netology/Diploma/SE/index.html";

std::string url_decode(const std::string &encoded) {
  std::string res;
  std::istringstream iss(encoded);
  char ch;

  while (iss.get(ch)) {
    if (ch == '%') {
      int hex;
      iss >> std::hex >> hex;
      res += static_cast<char>(hex);
    } else {
      res += ch;
    }
  }

  return res;
}

std::string convert_to_utf8(const std::string &str) { return url_decode(str); }

}

HttpConnection::HttpConnection(tcp::socket socket,
                               const SearcherConnection &searcherConnection)
    : socket_(std::move(socket)), searcherConnection_(searcherConnection) {}

void HttpConnection::start() {
  readRequest();
  checkDeadline();
}

void HttpConnection::readRequest() {
  auto self = shared_from_this();

  http::async_read(socket_, buffer_, request_,
                   [self](beast::error_code ec, std::size_t bytes_transferred) {
                     boost::ignore_unused(bytes_transferred);
                     if (!ec)
                       self->handleRequest();
                   });
}

void HttpConnection::handleRequest() {
  response_.version(request_.version());
  response_.keep_alive(false);

  switch (request_.method()) {
  case http::verb::get:
    response_.result(http::status::ok);
    response_.set(http::field::server, "Beast");
    createResponseGet();
    break;
  case http::verb::post:
    response_.result(http::status::ok);
    response_.set(http::field::server, "Beast");
    createResponsePost();
    break;

  default:
    response_.result(http::status::bad_request);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body())
        << "Invalid request-method '" << std::string(request_.method_string())
        << "'";
    break;
  }

  writeResponse();
}

void HttpConnection::createResponseGet() {
  if (request_.target() == "/") {
    response_.set(http::field::content_type, "text/html");
    std::ifstream fileHTML(htmlPath);
    if (fileHTML) {
      std::stringstream bufferHTML;
      bufferHTML << fileHTML.rdbuf();
      beast::ostream(response_.body()) << bufferHTML.str();
    }
  } else {
    response_.result(http::status::not_found);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body()) << "File not found\r\n";
  }
}

void HttpConnection::createResponsePost() {
  if (request_.target() == "/") {
    std::string s = buffers_to_string(request_.body().data());

    std::cout << "POST data: " << s << std::endl;

    size_t pos = s.find('=');
    if (pos == std::string::npos) {
      response_.result(http::status::not_found);
      response_.set(http::field::content_type, "text/plain");
      beast::ostream(response_.body()) << "File not found\r\n";
      return;
    }

    std::string key = s.substr(0, pos);
    std::string value = s.substr(pos + 1);

    std::string utf8value = convert_to_utf8(value);

    if (key != "search") {
      response_.result(http::status::not_found);
      response_.set(http::field::content_type, "text/plain");
      beast::ostream(response_.body()) << "File not found\r\n";
      return;
    }

    Searcher searcher(searcherConnection_);
    searcher.setSearchValue(value);
    std::vector<std::string> searchResult = searcher.getSearchResult();

    response_.set(http::field::content_type, "text/html");
    beast::ostream(response_.body())
        << "<html>\n"
        << "<head><meta charset=\"UTF-8\"><title>Buck Buck GO</title></head>\n"
        << "<body>\n"
        << "<h1>Buck Buck GO</h1>\n"
        << "<p>Response:<p>\n"
        << "<ul>\n";

    if (!searcher.isTablesExist()) {
      beast::ostream(response_.body()) << "Data base is empty!";
    } else if (searchResult.empty()) {
      beast::ostream(response_.body()) << "Not founded!";
    }

    for (const auto &url : searchResult) {

      beast::ostream(response_.body())
          << "<li><a href=\"" << url << "\">" << url << "</a></li>";
    }

    beast::ostream(response_.body()) << "</ul>\n"
                                     << "</body>\n"
                                     << "</html>\n";
  } else {
    response_.result(http::status::not_found);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body()) << "File not found\r\n";
  }
}

void HttpConnection::writeResponse() {
  auto self = shared_from_this();

  response_.content_length(response_.body().size());

  http::async_write(socket_, response_,
                    [self](beast::error_code ec, std::size_t) {
                      self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                      self->deadline_.cancel();
                    });
}

void HttpConnection::checkDeadline() {
  auto self = shared_from_this();

  deadline_.async_wait([self](beast::error_code ec) {
    if (!ec) {
      self->socket_.close(ec);
    }
  });
}

}
