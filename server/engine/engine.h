#pragma once

#include <pqxx/pqxx>
#include <string>
#include <vector>

struct SearcherConnection {
  std::string host;
  std::string port;
  std::string dbname;
  std::string user;
  std::string password;
};

class Searcher {
public:
  Searcher(const SearcherConnection &searcherConnection);
  ~Searcher();
  void setSearchValue(const std::string value);
  std::vector<std::string> getSearchResult() const;
  bool isTablesExist() const;

private:
  SearcherConnection searcherConnection_;
  std::unique_ptr<pqxx::connection> c_;
  std::string value_;
  std::vector<std::string> searchResult_;
  bool isTablesExist_;
  const int searchResultCountMax_ = 10;

  void findSearchResults();
  void checkTablesExistance();
};
