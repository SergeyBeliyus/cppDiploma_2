#pragma once

#include "parser/parser.h"
#include "link_get/link_get.h"
#include "sql_database/sql_database.h"
#include <string>


class Indexer {
public:
  Indexer(const SqlDataConnection &sqlDataConnection);
  void setHtml(const std::string html);
  void setCurrentLink(const httputils::Link &link);
  std::vector<httputils::Link> getLinks() const;
  std::string getHandledHtml() const;
  void dropTables();

private:
  SqlDatabase sqlDatabase_;
  HtmlParser htmlParser_;
  LinksGetter linksGetter_;
  void handleHtml();
};
