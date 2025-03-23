#pragma once

#include <regex>
#include <string>

class HtmlParser {
public:
  HtmlParser();
  void setHtml(const std::string html);
  std::string getHandledHtml() const;

private:
  std::string html_;
  std::string htmlHandled_;
  std::regex htmlTags_;
  std::regex nonAlnum_;
  std::regex pattern_;
  void handleHtml();
};
