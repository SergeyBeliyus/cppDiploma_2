#include "engine.h"
#include <iostream>

Searcher::Searcher(const SearcherConnection &searcherConnection)
    : searcherConnection_(searcherConnection), value_(), searchResult_(),
      isTablesExist_(false) {
  try {
    c_ = std::make_unique<pqxx::connection>(
        "host=" + searcherConnection_.host + " " +
        "port=" + searcherConnection_.port + " " + "dbname=" +
        searcherConnection_.dbname + " " + "user=" + searcherConnection_.user +
        " " + "password=" + searcherConnection_.password + " ");
  } catch (std::exception &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
  }
}

Searcher::~Searcher() {}

void Searcher::setSearchValue(const std::string value) {
  value_ = value;
  findSearchResults();
}

std::vector<std::string> Searcher::getSearchResult() const {
  return searchResult_;
}

bool Searcher::isTablesExist() const { return isTablesExist_; }

void Searcher::findSearchResults() {
  checkTablesExistance();
  if (isTablesExist_) {
    int pos = 0;
    while ((pos = value_.find("+", pos)) != std::string::npos) {
      value_.replace(pos, 1, "', '");
      pos += 1;
    }

    pqxx::work tx{*c_};
    std::string insert = "SELECT document, COUNT(dw.word_id) FROM documents d "
                         "JOIN documents_words dw ON d.id = dw.document_id "
                         "JOIN words w ON dw.word_id = w.id "
                         "WHERE w.word IN ('" +
                         value_ +
                         "') "
                         "GROUP BY d.id "
                         "ORDER BY count DESC;";
    pqxx::result documents = tx.exec(insert);
    for (int i = 0; i < documents.size(); ++i) {
      std::string document = documents[i]["document"].as<std::string>();
      if (i < searchResultCountMax_) {
        searchResult_.push_back(document);
      } else {
        break;
      }
    }
    tx.commit();
  }
}

void Searcher::checkTablesExistance() {
  pqxx::work tx{*c_};
  std::string insert =
      "SELECT EXISTS ("
      "SELECT 1 "
      "FROM information_schema.tables "
      "WHERE table_schema = 'public' "
      "AND table_name IN ('documents', 'documents_words', 'words')"
      ");";
  pqxx::result tablesExist = tx.exec(insert);
  isTablesExist_ = tablesExist[0][0].as<bool>();
}
