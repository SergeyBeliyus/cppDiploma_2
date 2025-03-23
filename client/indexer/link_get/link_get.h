#pragma once

#include "../../utils/utils.h"
#include <regex>
#include <string>
#include <vector>
#include <boost/url.hpp>

class LinksGetter {
public:
  LinksGetter();
  void setHtml(const std::string html);
  void setCurrentLink(const httputils::Link &link);
  std::vector<httputils::Link> getLinks() const;
  std::string getURL();

private:
  std::string html_;
  httputils::Link link_;
  std::vector<httputils::Link> links_;
  std::regex htmlTag_;
  std::regex urlRegex_;
  void handleHtml();

  std::string resolveUrl(const std::string& baseUrl, const std::string& relativeUrl);
};
