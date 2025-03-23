#include "parser.h"
#include <sstream>

HtmlParser::HtmlParser()
    : html_(), htmlHandled_(), htmlTags_("<[^>]*>"), nonAlnum_("[^a-zA-Z0-9 ]"),
      pattern_("\\b\\w{1,2}\\b") {}

void HtmlParser::setHtml(const std::string html) {
  html_ = html;
  handleHtml();
}

std::string HtmlParser::getHandledHtml() const { return htmlHandled_; }

void HtmlParser::handleHtml() {
  htmlHandled_ = std::regex_replace(html_, htmlTags_, " ");
  htmlHandled_ = std::regex_replace(htmlHandled_, nonAlnum_, " ");
  htmlHandled_ = std::regex_replace(htmlHandled_, pattern_, "");

  std::istringstream iss(htmlHandled_);
  std::ostringstream oss;
  std::string word;
  while (iss >> word) {
    for (char &c : word) {
      c = std::tolower(c);
    }
    oss << word << " ";
  }
  htmlHandled_ = oss.str();
  htmlHandled_.pop_back();
}
