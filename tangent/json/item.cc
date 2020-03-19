// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "tangent/json/item.h"
#include <string>

namespace json {
namespace item {

void Group::append(Item* item) {
  if (tail_) {
    tail_->next = item;
    tail_ = item;
  } else {
    head_ = tail_ = item;
  }
}

void Item::assign_key(const re2::StringPiece& string) {
  this->typeno = JSON_KEY;
  this->store.string = string.substr(1, string.size() - 2);
}

void Item::operator=(const re2::StringPiece& string) {
  this->typeno = JSON_STRING;
  this->store.string = string;
}

void Item::operator=(double inval) {
  this->typeno = JSON_FLOAT;
  this->store.floatval = inval;
}

void Item::operator=(int64_t inval) {
  this->typeno = JSON_INTEGER;
  this->store.integer = inval;
}

void Item::operator=(bool inval) {
  this->typeno = JSON_BOOLEAN;
  this->store.boolean = inval;
}

void Item::operator=(std::nullptr_t) {
  this->typeno = JSON_NULL;
}

void Item::operator=(const List& list) {
  this->typeno = JSON_LIST;
  this->store.list = list;
}

void Item::operator=(const Object& object) {
  this->typeno = JSON_OBJECT;
  this->store.object = object;
}

Group* Item::as_group() {
  if (typeno == JSON_OBJECT) {
    return &(store.object);
  } else if (typeno == JSON_LIST) {
    return &(store.list);
  } else {
    return nullptr;
  }
}

Item kInvalidItem{};

Item* get_next(Item* item) {
  if (item) {
    return item->next;
  } else {
    return nullptr;
  }
}

const Item& Item::operator[](const char* query_key) const {
  if (typeno != JSON_OBJECT) {
    return kInvalidItem;
  }

  Item* key = store.object.head_;
  Item* value = get_next(key);

  while (key && value) {
    if (key->store.string == query_key) {
      return *value;
    }
    key = get_next(value);
    value = get_next(key);
  }

  return kInvalidItem;
}

const Item& Item::operator[](const std::string& key) const {
  return (*this)[key.c_str()];
}

const Item& Item::operator[](size_t query_idx) const {
  if (typeno != JSON_LIST) {
    return kInvalidItem;
  }

  Item* item = store.object.head_;
  for (size_t idx = 0; idx < query_idx; idx++) {
    item = get_next(item);
  }

  if (item) {
    return *item;
  } else {
    return kInvalidItem;
  }
}

int parse_token(const Token& token, Item* item) {
  memset(item, 0, sizeof(Item));

  switch (token.typeno) {
    case Token::STRING_LITERAL: {
      (*item) = token.spelling.substr(1, token.spelling.size() - 2);
      return 0;
    }

    case Token::BOOLEAN_LITERAL: {
      if (token.spelling == "true") {
        (*item) = true;
        return 0;
      }
      (*item) = false;
      return 0;
    }

    case Token::NULL_LITERAL: {
      (*item) = nullptr;
      return 0;
    }

    case Token::NUMERIC_LITERAL: {
      int64_t intval;
      if (RE2::FullMatch(token.spelling, "([-\\d]+)", &intval)) {
        (*item) = intval;
        return 0;
      }

      double floatval;
      if (RE2::FullMatch(token.spelling, "(.+)", &floatval)) {
        (*item) = floatval;
        return 0;
      }

      return -1;
    }

    default: {
      return -1;
    }
  }
}

ItemParser::ItemParser(Item* begin, Item* end)
    : mem_begin_(begin), mem_write_(begin), mem_end_(end) {}

Item* ItemParser::alloc_item(Error* error) {
  if (mem_write_ < mem_end_) {
    Item* item = (mem_write_++);
    memset(item, 0, sizeof(Item));
    return item;
  }

  return nullptr;
}

int ItemParser::consume(const Token& tok, Error* error) {
  Event event{};
  int err = handle_token(tok, &event, error);
  switch (err) {
    case -1:
    case 0:
      return err;
    default:
      break;
  }

  switch (event.typeno) {
    case Event::OBJECT_BEGIN: {
      Item* item = alloc_item(error);
      if (!item) {
        fmt_error(error, Error::PARSE_OOM, tok.location)
            << "Exceeded available item storage";
        return -2;
      }

      (*item) = Object{};

      if (item_stack_.size()) {
        item_stack_.back()->as_group()->append(item);
      }
      item_stack_.push_back(item);
      return 0;
    }

    case Event::LIST_BEGIN: {
      Item* item = alloc_item(error);
      if (!item) {
        fmt_error(error, Error::PARSE_OOM, tok.location)
            << "Exceeded available item storage";
        return -3;
      }

      (*item) = List{};

      if (item_stack_.size()) {
        item_stack_.back()->as_group()->append(item);
      }
      item_stack_.push_back(item);
      return 0;
    }

    case Event::VALUE_LITERAL: {
      Item* item = alloc_item(error);
      if (!item) {
        fmt_error(error, Error::PARSE_OOM, tok.location)
            << "Exceeded available item storage";
        return -4;
      }

      parse_token(tok, item);
      if (!item_stack_.size()) {
        fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected initial object ({}) or list ([]) but got "
            << tok.spelling;
        return -5;
      }

      item_stack_.back()->as_group()->append(item);
      return 0;
    }

    case Event::OBJECT_KEY: {
      if (tok.typeno != Token::STRING_LITERAL) {
        fmt_error(error, Error::PARSE_UNEXPECTED_TOKEN, tok.location)
            << "Expected a string literal (key) but got " << tok.spelling;
        return -6;
      }

      Item* item = alloc_item(error);
      if (!item) {
        return -7;
      }

      item->assign_key(tok.spelling);

      if (!item_stack_.size()) {
        fmt_error(error, Error::INTERNAL_ERROR, tok.location)
            << "item_stack_ is empty" << tok.spelling;
        return -8;
      }

      if (item_stack_.back()->typeno != Item::JSON_OBJECT) {
        fmt_error(error, Error::INTERNAL_ERROR, tok.location)
            << "item_stack_ is not an object" << tok.spelling;
        return -9;
      }

      item_stack_.back()->as_group()->append(item);
      return 0;
    }

    case Event::LIST_END: {
      item_stack_.pop_back();
      return 0;
    }

    case Event::OBJECT_END: {
      item_stack_.pop_back();
      return 0;
    }

    default:
      fmt_error(error, Error::INTERNAL_ERROR, tok.location)
          << "Unhandled parse event";
      return -10;
  }

  return 0;
}

}  // namespace item
}  // namespace json
