#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../ini_parser/ini_parser.h"
#include "indexer/indexer.h"
#include "utils/utils.h"
#include <functional>

namespace {

const std::string configPath =
    "C:/SE/config.ini";
const std::string StartPageSection = "Crowler.StartPage";
const std::string RecursionDepthSection = "Crowler.RecursionDepth";
const std::string HostSection = "SQLConnection.Host";
const std::string PortSection = "SQLConnection.Port";
const std::string DataBaseNameSection = "SQLConnection.DataBaseName";
const std::string UserSection = "SQLConnection.User";
const std::string PasswordSection = "SQLConnection.Password";

std::mutex mtx;
std::condition_variable cv;
std::queue<std::function<void()>> tasks;
bool exitThreadPool = false;

}

void threadPoolWorker() {
  std::unique_lock<std::mutex> lock(mtx);
  while (!exitThreadPool || !tasks.empty()) {
    if (tasks.empty()) {
      cv.wait(lock);
    } else {
      auto task = tasks.front();
      tasks.pop();
      lock.unlock();
      task();
      lock.lock();
    }
  }
}

httputils::Link convertUrlToLink(const std::string url) {
  std::string protocol;
  std::string hostName;
  std::string query;
  std::regex urlRegex(R"((https?)://([^/]+)(/.*)?$)");
  std::smatch urlMatch;

  if (std::regex_match(url, urlMatch, urlRegex)) {
    if (urlMatch.size() == 4) {
      protocol = urlMatch[1].str();
      hostName = urlMatch[2].str();
      query = urlMatch[3].str();
    }
  } else {
    throw "Wrong URL! Need URL format: http(s)://hostname/query";
  }

  httputils::Link link(httputils::setProtocolType(protocol), hostName, query);
  return link;
}

void parseLink(const httputils::Link &link,
               const SqlDataConnection &sqlDataConnection, int depth) {
  try {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string html = getHtmlContent(link);

    if (html.size() == 0) {
      std::cout << "Failed to get HTML Content" << std::endl;
      return;
    }

    Indexer indexer(sqlDataConnection);
    indexer.setCurrentLink(link);
    indexer.setHtml(html);
    std::vector<httputils::Link> links = indexer.getLinks();

    if (depth > 0) {
      std::lock_guard<std::mutex> lock(mtx);

      size_t count = links.size();
      size_t index = 0;
      for (auto &subLink : links) {
        tasks.push([subLink, sqlDataConnection, depth]() {
          parseLink(subLink, sqlDataConnection, depth - 1);
        });
      }
      cv.notify_one();
    }
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}

int main() {
  try {
    IniParser iniParser(configPath);
    int depth = 1;
    std::string startPage = "https://en.wikipedia.org/wiki/Main_Page";
    SqlDataConnection sqlDataConnection;
    try {
      depth = iniParser.getValue<int>(RecursionDepthSection);
      startPage = iniParser.getValue<std::string>(StartPageSection);

      sqlDataConnection.host = iniParser.getValue<std::string>(HostSection);
      sqlDataConnection.port = iniParser.getValue<std::string>(PortSection);
      sqlDataConnection.dbname =
          iniParser.getValue<std::string>(DataBaseNameSection);
      sqlDataConnection.user = iniParser.getValue<std::string>(UserSection);
      sqlDataConnection.password =
          iniParser.getValue<std::string>(PasswordSection);

    } catch (std::exception &ex) {
      std::cout << ex.what();
    }

    {
      Indexer indexer(sqlDataConnection);
      indexer.dropTables();
    }

    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threadPool;

    for (int i = 0; i < numThreads; ++i) {
      threadPool.emplace_back(threadPoolWorker);
    }

    try {
      httputils::Link link = convertUrlToLink(startPage);

      {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push([link, sqlDataConnection, depth]() {
          parseLink(link, sqlDataConnection, depth);
        });
        cv.notify_one();
      }
    } catch (char const *e) {
      std::cerr << e << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
      std::lock_guard<std::mutex> lock(mtx);
      exitThreadPool = true;
      cv.notify_all();
    }

    for (auto &t : threadPool) {
      t.join();
    }
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
