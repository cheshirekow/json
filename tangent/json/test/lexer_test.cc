// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>
#include <vector>

#include <gtest/gtest.h>

#include "tangent/json/json.h"

using namespace json;  // NOLINT

TEST(LexerTest, StringLiteralTest) {
  std::string test_string = "{\"foo\" : \"hello\"}";
  std::vector<Token> tokens;
  tokens.resize(10);

  Scanner scanner;
  Error err{{}, Error::NOERROR};
  ASSERT_EQ(0, scanner.init(&err));
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 7; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[7], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::WHITESPACE, tokens[2].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[3].typeno);
  EXPECT_EQ(Token::WHITESPACE, tokens[4].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[5].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[6].typeno);
}

TEST(LexerTest, NumericLiteralTest) {
  std::vector<Token> tokens;
  tokens.resize(10);
  Error err{{}, Error::NOERROR};

  Scanner scanner;
  ASSERT_EQ(0, scanner.init(&err));

  std::string test_string = "{\"foo\":1234}";
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":-1234}";
  err.code = Error::NOERROR;
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":12.34}";
  err.code = Error::NOERROR;
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":12.34e+10}";
  err.code = Error::NOERROR;
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":12.34e-10}";
  err.code = Error::NOERROR;
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::NUMERIC_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);
}

TEST(LexerTest, BooleanLiteralTest) {
  std::vector<Token> tokens;
  tokens.resize(10);

  std::string test_string;
  Error err;
  Scanner scanner;

  ASSERT_EQ(0, scanner.init(&err));

  test_string = "{\"foo\":true}";
  err.code = Error::NOERROR;
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::BOOLEAN_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);

  test_string = "{\"foo\":false}";
  err.code = Error::NOERROR;
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::BOOLEAN_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);
}

TEST(LexerTest, NullLiteralTest) {
  std::vector<Token> tokens;
  tokens.resize(10);

  std::string test_string;
  Error err;
  Scanner scanner;

  ASSERT_EQ(0, scanner.init(&err));

  test_string = "{\"foo\":null}";
  err.code = Error::NOERROR;
  ASSERT_EQ(0, scanner.begin(test_string));

  for (size_t idx = 0; idx < 5; ++idx) {
    re2::StringPiece capture = scanner.get_piece();
    ASSERT_EQ(0, scanner.pump(&tokens[idx], &err))
        << err.what() << " for token " << idx << " at \""
        << capture.substr(0, 10).ToString() << "\"";
  }

  ASSERT_EQ(-1, scanner.pump(&tokens[5], &err));
  ASSERT_EQ(Error::LEX_INPUT_FINISHED, err.code);
  EXPECT_EQ(Token::PUNCTUATION, tokens[0].typeno);
  EXPECT_EQ(Token::STRING_LITERAL, tokens[1].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[2].typeno);
  EXPECT_EQ(Token::NULL_LITERAL, tokens[3].typeno);
  EXPECT_EQ(Token::PUNCTUATION, tokens[4].typeno);
}

TEST(LexerTest, ErrorOnMalformed) {
  Error error;
  EXPECT_EQ(0, verify_lex("{\"foo\":\"bar\"}", &error))
      << "Error (" << error.code << "): " << error.msg;

  // Comments are not allowed
  Token tokens[10];
  ASSERT_GT(0, lex("{} #hello", &tokens, &error));
  ASSERT_GT(0, verify_lex("{} #hello", &error));
  EXPECT_EQ(Error::LEX_INVALID_TOKEN, error.code);
  ASSERT_GT(0, verify_lex("{} //hello", &error));
  EXPECT_EQ(Error::LEX_INVALID_TOKEN, error.code);

  // Incomplete string literal
  ASSERT_GT(0, verify_lex("{\"foo\" : \"hello", &error));
  EXPECT_EQ(Error::LEX_INVALID_TOKEN, error.code);

  // Invalid numeric literal
  // NOTE(josh): the string is valid up through
  // "{\n\"foo\" : 1,\n\"bar\": 12.3"
  ASSERT_GT(0, verify_lex("{\n\"foo\" : 1,\n\"bar\": 12.3.4}", &error));
  EXPECT_EQ(Error::LEX_INVALID_TOKEN, error.code);
  EXPECT_EQ(2, error.loc.lineno);
  EXPECT_EQ(11, error.loc.colno);
  EXPECT_EQ(24, error.loc.offset);
}
