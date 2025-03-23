#include "link_get.h"

LinksGetter::LinksGetter()
    : html_(), link_(httputils::ProtocolType::HTTPS, "", ""), links_(),
      htmlTag_(R"(<a href="([^"]*))"),
      urlRegex_(R"((https?)://([^/]+)(/.*)?$)") {}

void LinksGetter::setHtml(const std::string html) {
  html_ = html;
  handleHtml();
}

void LinksGetter::setCurrentLink(const httputils::Link &link) { link_ = link; }

void LinksGetter::handleHtml() {
  std::smatch match;
  std::string::const_iterator searchStart(html_.cbegin());
  while (std::regex_search(searchStart, html_.cend(), match, htmlTag_)) {
    std::string url = match[1].str();

    // Удаляем ссылки, начинающиеся с "javascript:"
    if (url.find("javascript:") == 0) {
      searchStart = match.suffix().first;
      continue;
    }

    // Преобразуем относительные ссылки в абсолютные
    if (url.find("http") != 0) { // Если ссылка не начинается с "http"
      url = resolveUrl(getURL(), url);
    }

    // Проверяем, что ссылка корректна
    std::smatch urlMatch;
    if (std::regex_match(url, urlMatch, urlRegex_)) {
      std::string protocol = urlMatch[1].str();
      std::string hostName = urlMatch[2].str();
      std::string path = urlMatch[3].str();

      if (path.empty()) {
        path = "/";
      }

      httputils::Link link(httputils::setProtocolType(protocol), hostName, path);
      links_.push_back(link);
    }

    searchStart = match.suffix().first;
  }
}

std::vector<httputils::Link> LinksGetter::getLinks() const { return links_; }

std::string LinksGetter::getURL() {
  const std::string URL =
      getProtocol(link_.protocol) + "://" + link_.hostName + link_.query;
  return URL;
}

std::string LinksGetter::resolveUrl(const std::string& baseUrl, const std::string& relativeUrl) {
    boost::urls::url base(baseUrl);
    boost::urls::url relative(relativeUrl);
    boost::urls::url result;                // Объект для результата
    
    boost::urls::resolve(base, relative, result); // Разрешение URL
    
    return result.buffer(); // Возвращаем строку результата
}
