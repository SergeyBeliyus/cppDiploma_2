#include "indexer.h"
#include <iostream>

Indexer::Indexer(const SqlDataConnection &sqlDataConnection)
    : sqlDatabase_(sqlDataConnection), htmlParser_(), linksGetter_() {}

void Indexer::setCurrentLink(const httputils::Link &link) {
  linksGetter_.setCurrentLink(link);
  sqlDatabase_.setURL(linksGetter_.getURL());
}

void Indexer::setHtml(const std::string html) {
  linksGetter_.setHtml(html);
  htmlParser_.setHtml(html);
  handleHtml();
}

void Indexer::handleHtml() {
  const std::string handledHtml = htmlParser_.getHandledHtml();
  std::istringstream iss(handledHtml);
  std::string word;

  while (iss >> word) {
    sqlDatabase_.setWord(word);
  }
  sqlDatabase_.addIds();
}

std::vector<httputils::Link> Indexer::getLinks() const {
  return linksGetter_.getLinks();
}

std::string Indexer::getHandledHtml() const {
  return htmlParser_.getHandledHtml();
}

void Indexer::dropTables() { sqlDatabase_.dropTables(); }
