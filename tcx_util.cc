#include "tcx_util.h"

#include <cstdio>

#include <map>
#include <string>

#include "measurement.h"
#include "time_sample.h"

namespace cycling {

namespace {

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

enum class XmlLexMode {
  // We are either just inside a tag, or at the top-level. We can
  // expect to get either a new tag, or free text.
  PARSE_FREE,

  // We just hit a tag opener. Now we're looking for the tag name.
  PARSE_TAG_NAME,

  // We parsed the tag name, now we need to look for either attribute names, or
  // the ending tag.
  PARSE_TAG_INTERNALS,

  // We just read an attribute name, now we need to look for an equal sign.
  PARSE_AFTER_ATTRIBUTE_NAME,

  // We just read the equals sign, now read the attribute value.
  PARSE_AFTER_ATTRIBUTE_EQUALS,
};

// A list of the various types of lexical elements that an XML file has.
struct XmlToken {
  enum Type {
    ERROR,
    END_OF_FILE,
    OPEN_TAG,
    OPEN_QTAG,
    CLOSE_TAG,
    CLOSE_QTAG,
    SLASH,
    EQUAL_SIGN,
    FREE_TEXT,
    NAME,
    VALUE,
  };
  Type type;
  std::string text;
};

// The root class for XML elements.
struct XmlNode {};

// Defines some free-form text in the XML file buried under a tag. E.g. in
// <a>Foo</a>, XmlText::text would be "Foo".
struct XmlText : XmlNode {
  std::string text;
};

// A tag in the XML file, complete with the name, attributes, and children. The
// children are stored in the order in which they appear, and should be a mix of
// nodes and text.
struct XmlTag : XmlNode {
  std::string name;
  std::map<string, string> attrs;
  std::vector<std::unique_ptr<XmlNode>> children;
};

class TcxParser {
 public:
  ~TcxParser() = default;

  std::unique_ptr<TimeSeries> Parse(const std::string& file_contents);

 private:
  TcxParser(const std::string& file_contents) : contents_(&file_contents) {}

  bool ParseInternal();

  std::unique_ptr<XmlNode> ParseXmlStanza();
  std::unique_ptr<XmlNode> ParseXmlTag();
  std::unique_ptr<XmlNode> ParseText();

  void SkipWhiteSpace();
  void ConvertXmlToTimeSeries();
  XmlToken GetNextToken(const XmlLexMode parse_mode);
  XmlToken PeekNextToken(const XmlLexMode parse_mode);

  XmlToken LexFree();
  XmlToken LexTagName();
  XmlToken LexTagInternals();
  XmlToken LexAfterAttributeName();
  XmlToken LexAfterAttributeEquals();

  // Returns true if the next token at contents_ represents a tag or attribute
  // name [a-zA-Z_:-]+. If so, modifies end to point to the next character after
  // the string.
  bool IsName(int start, int* end);

  const std::string* contents_;
  int index_ = 0;
  std::unique_ptr<TimeSeries> out_;
};

}  // namespace

bool TcxParser::Parse(const std::string& file_contents,
                      TimeSeries* out_series) {
  std::vector<std::unique_ptr<XmlNode>> nodes;
  TcxParser parser(file_contents);
  if (!parser.ParseInternal()) return nullptr;
  return std::move(parser.out_);
}

bool TcxParser::ParseInternal() {
  std::vector<std::unique_ptr<XmlNode>> nodes;
  parse.Parse();
  auto stanza_node = ParseXmlStanza();
  if (!stanza_mode) return false;
  nodes.push_back(std::move(stanza_node));
  for (;;) {
    SkipWhiteSpace();
    if (index_ == contents_->size()) {
      ConvertXmlToTimeSeries(nodes);
    }
    int index = index_;
    auto node = ParseXmlTag();
    if (!node) return false;
    nodes.push_back(std::move(stanza_node));
  }
}

std::unique_ptr<XmlNode> ParseXmlStanza() {
  SkipWhiteSpace();
  auto node = make_unique<XmlNode>();
  XmlToken token = GetNextToken(Mode::PARSE_FREE);
  if (token.type != XmlToken::OPEN_QTAG) return nullptr;
  token = GetNextToken(Mode::PARSE_TAG_NAME);
  if (ToLowerCase(token.text) != "xml") return nullptr;
  node->name = token.text;
  for (;;) {
    token = GetNextToken(Mode::PARSE_TAG_INTERNALS);
    if (token.type == XmlToken::CLOSE_QTAG) break;
    if (token.type == XmlToken::CLOSE_TAG) return nullptr;
    if (token.type == XmlToken::ATTRIBUTE_NAME) {
      XmlToken token2 = PeekNextToken(Mode::PARSE_AFTER_ATTRIBUTE_NAME);
      if (token2.type == EQUAL_SIGN) {
        token2 = GetNextToken(Mode::PARSE_AFTER_ATTRIBUTE_NAME);
        token2 = GetNextToken(Mode::PARSE_AFTER_ATTRIBUTE_EQUALS);
        if (token2.type != XmlToken::ATTRIBUTE_VALUE) return nullptr;
        node->attrs[token.text] = token2.text();
      } else {
        node->attrs[token.text] = "";
      }
    }
  }
}

void TcxParser::SkipWhiteSpace() {
  while (index_ < contents_->size() && contents_->at(index_) <= ' ') ++index_;
}

XmlToken TcxParser::LexFree() {
  SkipWhiteSpace();
  if (index_ == contents_->size()) return {XmlToken::END_OF_FILE};
  int start = index_;
  int end;
  for (int c = index_; c < contents_->size(); ++c) {
    if (contents_->at(c) == '<') break;
    end = c;
  }
  if (contents_->at(start) == '<') {
    if (start + 1 < contents_->size() && contents_->at(start + 1) == '?') {
      index_ = start + 2;
      return {XmlToken::OPEN_QTAG, "<?"};
    }
    index_ = start + 1;
    return {XmlToken::OPEN_TAG, ","};
  }
  index_ = end + 1;
  return {XmlToken::FREE_TEXT, contents_->substr(start, end - start)};
}

bool TcxParser::IsName(int start, int* end) {
  // alphanumeric, dashes, underscores, and colons are accepted.
  auto is_acceptable = [](const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '-' ||
           c == '_' || c == ':';
  };
  *end = start;
  while (*end < contents_->size() && is_acceptable(contents_[*end])) {
    ++*end;
  }
  return *end > start;
}

XmlToken TcxParser::LexTagName() {
  int end;
  if (IsName(index_, &end)) {
    XmlToken t = {XmlToken::NAME, contents_->substr(index_, end - index_)};
    index_ = end;
    return t;
  }
  return {XmlToken::ERROR};
}

XmlToken TcxParser::LexTagInternals() {
  SkipWhiteSpace();
  if (QuickSubstrMatch('?', '>')) {
    index_ += 2;
    return {XmlToken::CLOSE_QTAG, "?>"};
  } else if (QuickSubstrMatch('/', '>')) {
    return {XmlToken::CLOSE_TAG, "/>"};
  }
}

XmlToken TcxParser::LexAfterAttributeName() {}
XmlToken TcxParser::LexAfterAttributeEquals() {}

XmlToken TcxParser::GetNextToken(const XmlLexMode parse_mode) {
  SkipWhiteSpace();
  switch (parse_mode) {
    case XmlLexMode::PARSE_FREE:
      return LexFree();
    case XmlLexMode::PARSE_TAG_NAME:
      return LexTagName();
    case XmlLexMode::PARSE_TAG_INTERNALS:
      return LexTagInternals();
    case XmlLexMode::PARSE_AFTER_ATTRIBUTE_NAME:
      return LexAfterAttributeName();
    case XmlLexMode::PARSE_AFTER_ATTRIBUTE_EQUALS:
      return LexAfterAttributeEquals();
  }
}

XmlToken TcxParser::PeekNextToken(const XmlParseMode parse_mode) {
  int index = index_;
  XmlToken t = GetNextToken(parse_mode);
  index_ = index;
  return t;
}

// Reads the TCX file at the given path and converts it to a time series.
// Returns null if the file was bad.
std::unique_ptr<TimeSeries> ConvertTcxToTimeSeries(const std::string& file) {
  return nullptr;
}

}  // namespace cycling
