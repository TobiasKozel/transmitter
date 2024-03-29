#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <functional>

namespace transmitter {
  /**
   * Ripped from
   * https://stackoverflow.com/a/2616217
   * Pimped slightly to also output port and validity
   */
  struct URLParser {
    std::string protocol, host, path, query;
    int port = 0;
    bool ssl = false;
    bool valid = false;
    bool nonDefaultPort = false;
    URLParser(const std::string& url_s) {
      const std::string prot_end("://");
      std::string::const_iterator prot_i = search(url_s.begin(), url_s.end(),
        prot_end.begin(), prot_end.end()
      );

      protocol.reserve(distance(url_s.begin(), prot_i));

      transform(url_s.begin(), prot_i,
        back_inserter(protocol),
        std::ptr_fun<int, int>(tolower)
      ); // protocol is icase

      if (protocol == "https") {
        port = 443;
        ssl = true;
      }
      else if (protocol == "http") {
        port = 80;
        ssl = false;
      }
      else { return; } // No protocol

      if (prot_i == url_s.end()) { return; }

      advance(prot_i, prot_end.length());

      std::string::const_iterator path_i = find(prot_i, url_s.end(), '/');

      host.reserve(distance(prot_i, path_i));

      transform(prot_i, path_i,
        back_inserter(host),
        std::ptr_fun<int, int>(tolower)
      ); // host is icase

      const auto portIndex = host.find(':');
      if (portIndex != std::string::npos) {
        try {
          port = std::stoi(host.c_str() + portIndex + 1);
          host = host.substr(0, portIndex);
          nonDefaultPort = true;
        }
        catch (...) {
          return;
        }
      }

      std::string::const_iterator query_i = find(path_i, url_s.end(), '?');

      path.assign(path_i, query_i);

      if (query_i != url_s.end()) {
        ++query_i;
      }
      query.assign(query_i, url_s.end());

      valid = true;
    }

    /**
     * Returns the url with only the parts needed
     */
    std::string reconstruct(bool _proto, bool _path, bool _query) const {
      std::string ret;
      if (_proto) {
        ret = protocol + "://";
      }
      ret += host;

      if (nonDefaultPort) {
        ret += ":" + std::to_string(port);;
      }

      if (_path) {
        ret += path;
        if (_query && !query.empty()) {
          ret += "?" + query;
        }
      }
      return ret;
    }
  };
}