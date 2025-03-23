#include "utils.h"

#include <iostream>
#include <regex>

#include <boost/url.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <openssl/ssl.h>

namespace httputils {

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ip = boost::asio::ip;
namespace ssl = boost::asio::ssl;

using tcp = boost::asio::ip::tcp;

bool isText(const boost::beast::multi_buffer::const_buffers_type &b) {
  for (auto itr = b.begin(); itr != b.end(); itr++) {
    for (int i = 0; i < (*itr).size(); i++) {
      if (*((const char *)(*itr).data() + i) == 0) {
        return false;
      }
    }
  }

  return true;
}

std::string getHtmlContent(const Link &link) {

  std::string result;
  std::cout << "getHtmlContent" << std::endl;

  try {
    std::string host = link.hostName;
    std::string query = link.query;

    net::io_context ioc;

    if (link.protocol == ProtocolType::HTTPS) {

      ssl::context ctx(ssl::context::tlsv13_client);
      ctx.set_default_verify_paths();

      beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
      stream.set_verify_mode(ssl::verify_none);

      stream.set_verify_callback(
          [](bool preverified, ssl::verify_context &ctx) {
            return true; // Accept any certificate
          });

      if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()),
                             net::error::get_ssl_category()};
        throw beast::system_error{ec};
      }

      ip::tcp::resolver resolver(ioc);
      get_lowest_layer(stream).connect(resolver.resolve(host, "https"));
      get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

      http::request<http::empty_body> req{http::verb::get, query, 11};
      req.set(http::field::host, host);
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

      stream.handshake(ssl::stream_base::client);
      http::write(stream, req);

      beast::flat_buffer buffer;
      http::response<http::dynamic_body> res;
      http::read(stream, buffer, res);

      // Обработка редиректов
      if (res.result() == http::status::moved_permanently || res.result() == http::status::found) {
        auto location = res.find(http::field::location);
        if (location != res.end()) {
           std::string newUrl = location->value();

           // Разбираем новый URL с помощью Boost.URL
           boost::urls::url parsedUrl(newUrl);
           ProtocolType newProtocol = (parsedUrl.scheme() == "https") ? ProtocolType::HTTPS : ProtocolType::HTTP;
           std::string newHost = parsedUrl.host();
           std::string newQuery = parsedUrl.path() + (parsedUrl.has_query() ? "?" + std::string(parsedUrl.query()) : "");

           // Создаем новый объект Link с использованием конструктора
           Link newLink(newProtocol, newHost, newQuery);

           return getHtmlContent(newLink); // Рекурсивный вызов с новым Link
        }
      }

      if (isText(res.body().data())) {
        result = buffers_to_string(res.body().data());
      } else {
        std::cout << "This is not a text link, bailing out..." << std::endl;
      }

      beast::error_code ec;
      stream.shutdown(ec);
      if (ec == net::error::eof) {
        ec = {};
      }

      if (ec) {
        throw beast::system_error{ec};
      }
    } else {
      tcp::resolver resolver(ioc);
      beast::tcp_stream stream(ioc);

      auto const results = resolver.resolve(host, "http");

      stream.connect(results);

      http::request<http::string_body> req{http::verb::get, query, 11};
      req.set(http::field::host, host);
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

      http::write(stream, req);

      beast::flat_buffer buffer;

      http::response<http::dynamic_body> res;

      http::read(stream, buffer, res);

      if (isText(res.body().data())) {
        result = buffers_to_string(res.body().data());
      } else {
        std::cout << "This is not a text link, bailing out..." << std::endl;
      }

      beast::error_code ec;
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);

      if (ec && ec != beast::errc::not_connected)
        throw beast::system_error{ec};
    }
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  return result;
}

std::string getProtocol(const httputils::ProtocolType &protocolType) {
  std::string protocol;
  switch (protocolType) {
  case httputils::HTTP:
    protocol = "http";
    break;
  default:
    protocol = "https";
    break;
  }
  return protocol;
}

httputils::ProtocolType setProtocolType(const std::string protocol) {
  httputils::ProtocolType protocolType;
  if (protocol == "http") {
    protocolType = httputils::HTTP;

  } else if (protocol == "https") {
    protocolType = httputils::HTTPS;

  } else {
  }
  return protocolType;
}

}
