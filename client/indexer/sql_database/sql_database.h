#pragma once

#include <map>
#include <pqxx/pqxx>
#include <string>

struct SqlDataConnection {
  std::string host;
  std::string port;
  std::string dbname;
  std::string user;
  std::string password;
};

class SqlDatabase {
public:
  SqlDatabase(const SqlDataConnection &sqlDataConnection);
  ~SqlDatabase();
  void setURL(const std::string URL);
  void setWord(const std::string word);
  void addIds();
  void dropTables();

private:
  SqlDataConnection sqlDataConnection_;
  std::unique_ptr<pqxx::connection> c_;
  std::string URL_;
  std::map<std::string, std::vector<std::string>> documentsWords_;
  std::map<std::string, int> wordsCounts_;
  void createTables();
  void createTableDocuments();
  void createTableWords();
  void createTableDocumentsWords();
  void findWordsCounts();
};
