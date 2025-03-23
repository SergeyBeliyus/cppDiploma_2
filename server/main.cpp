#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <iostream>
#include <string>

#include "../ini_parser/ini_parser.h"
#include "connection.h"

namespace {

const std::string configPath =
    "C:/SE/config.ini";
const std::string SearchEnginePortSection = "SearchEngine.Port";
const std::string HostSection = "SQLConnection.Host";
const std::string PortSection = "SQLConnection.Port";
const std::string DataBaseNameSection = "SQLConnection.DataBaseName";
const std::string UserSection = "SQLConnection.User";
const std::string PasswordSection = "SQLConnection.Password";

void httpServer(tcp::acceptor &acceptor, tcp::socket &socket,
                const SearcherConnection &searcherConnection) {
  acceptor.async_accept(socket, [&](beast::error_code ec) {
    if (!ec)
      std::make_shared<httpsrvr::HttpConnection>(std::move(socket),
                                                 searcherConnection)
          ->start();
    httpServer(acceptor, socket, searcherConnection);
  });
}

}

int main(int argc, char *argv[]) {
  try {
    SearcherConnection searcherConnection;

    IniParser iniParser(configPath);
    unsigned short port = 8081;
    try {
      port = static_cast<unsigned short>(
          iniParser.getValue<int>(SearchEnginePortSection));

      searcherConnection.host = iniParser.getValue<std::string>(HostSection);
      searcherConnection.port = iniParser.getValue<std::string>(PortSection);
      searcherConnection.dbname =
          iniParser.getValue<std::string>(DataBaseNameSection);
      searcherConnection.user = iniParser.getValue<std::string>(UserSection);
      searcherConnection.password =
          iniParser.getValue<std::string>(PasswordSection);
    } catch (std::exception &ex) {
      std::cout << ex.what();
    }
    auto const address = net::ip::make_address("0.0.0.0");

    net::io_context ioc{1};

    tcp::acceptor acceptor{ioc, {address, port}};
    tcp::socket socket{ioc};
    httpServer(acceptor, socket, searcherConnection);

    std::cout << "Open browser and connect to http://localhost:" << port
              << " to see the web server operating" << std::endl;

    ioc.run();
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
