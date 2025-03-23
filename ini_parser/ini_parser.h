#pragma once

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class IniParser {
public:
  IniParser(const std::string filename);
  IniParser(IniParser &other) = delete;
  IniParser(IniParser &&other) = delete;
  IniParser &operator=(IniParser &other) = delete;
  IniParser &operator=(IniParser &&other) = delete;

  template <class T> T getValue(const std::string input_section_value);

  ~IniParser() { fin.close(); }

private:
  std::ifstream fin;
  std::map<std::string, std::map<std::string, std::string>> sections;
  void iniParserError(std::string &input_section_name,
                      std::string &input_value_name);
  void iniParserTypeError(
      std::string &input_section_name,
      std::map<std::string, std::map<std::string, std::string>> &sections);
};

IniParser::IniParser(const std::string filename) {
  fin.open(filename);
  if (!fin.is_open()) {
    throw std::runtime_error("file open failed");
  }
  std::string section_name;
  while (!(fin.eof())) {
    std::string value_name = "empty";
    std::string value = "empty";
    std::string current_string = "empty";
    std::getline(fin, current_string);
    if (std::count(current_string.begin(), current_string.end(), ' ') ==
        current_string.size()) {
      continue;
    }

    if (current_string.find("[") == 0 &&
        current_string.find("]") != std::string::npos) {
      current_string.resize(current_string.find(']') + 1);
      section_name = current_string.substr(1, current_string.size() - 2);
    } else {
      if (current_string.find(";") != std::string::npos &&
          current_string.find(";") != 0 && current_string.at(0) != ' ') {
        current_string.resize(current_string.find(';'));
        value_name = current_string.substr(0, current_string.find('='));
        value_name.erase(std::remove(value_name.begin(), value_name.end(), ' '),
                         value_name.end());
        value = current_string.substr(current_string.find('=') + 1,
                                      current_string.size() -
                                          value_name.size() - 1);
        sections[section_name][value_name] = value;
      } else if (current_string.find(";") == std::string::npos) {
        value_name = current_string.substr(0, current_string.find('='));
        value_name.erase(std::remove(value_name.begin(), value_name.end(), ' '),
                         value_name.end());
        value = current_string.substr(current_string.find('=') + 1,
                                      current_string.size() -
                                          value_name.size() - 1);
        if (value.size() == 0) {
          value = "";
        }
        sections[section_name][value_name] = value;
      } else {
        continue;
      }
    }

    if (value_name == "empty") {
      sections[section_name];
    }
  }
}

void IniParser::iniParserError(std::string &input_section_name,
                               std::string &input_value_name) {

  if (sections.count(input_section_name) == 0) {
    std::string section_error =
        "In INI file no input section. Look at sections.\n";
    for (const auto &el1 : sections) {
      section_error += '[' + el1.first + ']' + '\n';
    }
    throw std::runtime_error(section_error);
  } else {

    if (sections[input_section_name].count(input_value_name) == 0) {
      std::string value_error = "In INI file no input value. Look at [" +
                                input_section_name + "] content.\n[" +
                                input_section_name + "]\n";
      for (const auto &el2 : sections[input_section_name]) {
        value_error += el2.first + '=' + el2.second + '\n';
      }
      throw std::runtime_error(value_error);
    }
  }
}

void IniParser::iniParserTypeError(
    std::string &input_section_name,
    std::map<std::string, std::map<std::string, std::string>> &sections) {
  std::string type_error = "Invalid value type. Look at [" +
                           input_section_name + "] content.\n[" +
                           input_section_name + "]\n";
  for (const auto &el2 : sections[input_section_name]) {
    type_error += el2.first + '=' + el2.second + '\n';
  }
  throw std::runtime_error(type_error);
}

template <> int IniParser::getValue(const std::string input_section_value) {
  std::string input_section_name =
      input_section_value.substr(0, input_section_value.find('.'));
  std::string input_value_name = input_section_value.substr(
      input_section_value.find('.') + 1,
      input_section_value.size() - input_section_name.size() - 1);
  int result_value;
  iniParserError(input_section_name, input_value_name);
  try {
    result_value = stoi(sections[input_section_name][input_value_name]);
  } catch (...) {
    iniParserTypeError(input_section_name, sections);
  }
  return result_value;
}

template <> std::string IniParser::getValue(std::string input_section_value) {
  std::string input_section_name =
      input_section_value.substr(0, input_section_value.find('.'));
  std::string input_value_name = input_section_value.substr(
      input_section_value.find('.') + 1,
      input_section_value.size() - input_section_name.size() - 1);
  std::string result_value;
  iniParserError(input_section_name, input_value_name);
  result_value = sections[input_section_name][input_value_name];
  return result_value;
}

template <> double IniParser::getValue(std::string input_section_value) {
  std::string input_section_name =
      input_section_value.substr(0, input_section_value.find('.'));
  std::string input_value_name = input_section_value.substr(
      input_section_value.find('.') + 1,
      input_section_value.size() - input_section_name.size() - 1);
  double result_value;
  iniParserError(input_section_name, input_value_name);

  try {
    result_value = stod(sections[input_section_name][input_value_name]);
  } catch (...) {
    iniParserTypeError(input_section_name, sections);
  }
  return result_value;
}

template <> float IniParser::getValue(std::string input_section_value) {
  std::string input_section_name =
      input_section_value.substr(0, input_section_value.find('.'));
  std::string input_value_name = input_section_value.substr(
      input_section_value.find('.') + 1,
      input_section_value.size() - input_section_name.size() - 1);
  float result_value;
  iniParserError(input_section_name, input_value_name);
  try {
    result_value = stof(sections[input_section_name][input_value_name]);
  } catch (...) {
    iniParserTypeError(input_section_name, sections);
  }
  return result_value;
}
