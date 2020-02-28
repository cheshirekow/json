#include "json/json.h"

namespace json {

inline int Lex(const re2::StringPiece& source, std::vector<Token>* buf,
               Error* error = nullptr) {
  int ntokens = Lex(source, &(*buf)[0], buf->size(), error);
  if (ntokens > buf->size()) {
    buf->resize(ntokens);
    return Lex(source, &(*buf)[0], buf->size(), error);
  }
  return ntokens;
}

inline int Parse(const re2::StringPiece& source, std::vector<Event>* buf,
                 Error* error = nullptr) {
  int nevents = Parse(source, &(*buf)[0], buf->size(), error);
  if (nevents > buf->size()) {
    buf->resize(nevents);
    return Parse(source, &(*buf)[0], buf->size(), error);
  }
  return nevents;
}

}  // namespace json