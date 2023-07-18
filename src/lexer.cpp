#include "lexer.h"

#include <cassert>
#include <iostream>
#include <sstream>


namespace atc {

std::ostream& operator<<(std::ostream& o, const Location& l)
{
  return o << l.d_line << ":" << l.d_column;
}
std::ostream& operator<<(std::ostream& o, const Span& l)
{
  return o << l.d_start << "-" << l.d_end;
}

Lexer::Lexer()
    : d_bufferPos(0), d_bufferEnd(0), d_peekedChar(false), d_chPeeked(0)
{
  for (char ch = 'a'; ch <= 'z'; ++ch)
  {
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::SYMBOL_START);
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::SYMBOL);
  }
  for (char ch = 'a'; ch <= 'f'; ++ch)
  {
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::HEXADECIMAL_DIGIT);
  }
  for (char ch = 'A'; ch <= 'Z'; ++ch)
  {
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::SYMBOL_START);
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::SYMBOL);
  }
  for (char ch = 'A'; ch <= 'F'; ++ch)
  {
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::HEXADECIMAL_DIGIT);
  }
  for (char ch = '0'; ch <= '9'; ++ch)
  {
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::HEXADECIMAL_DIGIT);
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::DECIMAL_DIGIT);
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::SYMBOL);
  }
  d_charClass['0'] |= static_cast<uint32_t>(CharacterClass::BIT);
  d_charClass['1'] |= static_cast<uint32_t>(CharacterClass::BIT);
  // ~!@$%^&*_-+|=<>.?/
  for (char ch : s_extraSymbolChars)
  {
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::SYMBOL_START);
    d_charClass[ch] |= static_cast<uint32_t>(CharacterClass::SYMBOL);
  }
  // whitespace
  d_charClass[' '] |= static_cast<uint32_t>(CharacterClass::WHITESPACE);
  d_charClass['\t'] |= static_cast<uint32_t>(CharacterClass::WHITESPACE);
  d_charClass['\r'] |= static_cast<uint32_t>(CharacterClass::WHITESPACE);
  d_charClass['\n'] |= static_cast<uint32_t>(CharacterClass::WHITESPACE);
}

void Lexer::warning(const std::string& msg)
{
  std::cout << d_inputName << ':' << d_span.d_start.d_line << '.'
            << d_span.d_start.d_column << ": " << msg << std::endl;
}

void Lexer::parseError(const std::string& msg, bool eofException)
{
  std::stringstream os;
  if( d_span.d_start.d_line > 0 ) {
    os <<  "Parse Error: " << d_inputName << ":" << d_span.d_start.d_line << "."
        << d_span.d_start.d_column << ": " << msg;
  } else {
    os << "Parse Error: " << msg;
  }
  std::cerr << os.str();
  exit(1);
}

void Lexer::initSpan()
{
  d_span.d_start.d_line = 1;
  d_span.d_start.d_column = 0;
  d_span.d_end.d_line = 1;
  d_span.d_end.d_column = 0;
}

void Lexer::initialize(Input* input, const std::string& inputName)
{
  //Assert(input != nullptr);
  d_istream = input->getStream();
  d_isInteractive = input->isInteractive();
  d_inputName = inputName;
  initSpan();
  d_peeked.clear();
  d_bufferPos = 0;
  d_bufferEnd = 0;
  d_peekedChar = false;
  d_chPeeked = 0;
}

Token Lexer::nextToken()
{
  if (d_peeked.empty())
  {
    // Call the derived yylex() and convert it to a token
    return nextTokenInternal();
  }
  Token t = d_peeked.back();
  d_peeked.pop_back();
  return t;
}

Token Lexer::peekToken()
{
  // parse next token
  Token t = nextTokenInternal();
  // reinsert it immediately
  reinsertToken(t);
  // return it
  return t;
}

void Lexer::reinsertToken(Token t) { d_peeked.push_back(t); }

void Lexer::unexpectedTokenError(Token t, const std::string& info)
{
  //Assert(d_peeked.empty());
  std::ostringstream o{};
  o << info << ", got `" << tokenStr() << "` (" << t << ").";
  // Note that we treat this as an EOF exception if the token is EOF_TOK.
  // This is important for exception handling in interactive mode.
  parseError(o.str(), t == Token::EOF_TOK);
}

void Lexer::eatToken(Token t)
{
  Token tt = nextToken();
  if (t != tt)
  {
    std::ostringstream o{};
    o << "Expected a " << t;
    unexpectedTokenError(tt, o.str());
  }
}

bool Lexer::eatTokenChoice(Token t, Token f)
{
  Token tt = nextToken();
  if (tt == t)
  {
    return true;
  }
  else if (tt != f)
  {
    std::ostringstream o{};
    o << "Expected " << t << " or " << f;
    unexpectedTokenError(tt, o.str());
  }
  return false;
}


const char* Lexer::tokenStr() const
{
  //Assert(!d_token.empty() && d_token.back() == 0);
  return d_token.data();
}

Token Lexer::nextTokenInternal()
{
  //Trace("lexer-debug") << "Call nextToken" << std::endl;
  d_token.clear();
  Token ret = computeNextToken();
  // null terminate?
  d_token.push_back(0);
  //Trace("lexer-debug") << "Return nextToken " << ret << " / " << tokenStr() << std::endl;
  return ret;
}

Token Lexer::computeNextToken()
{
  bumpSpan();
  char ch;
  // skip whitespace and comments
  for (;;)
  {
    do
    {
      if ((ch = nextChar()) == EOF)
      {
        return Token::EOF_TOK;
      }
    } while (isCharacterClass(ch, CharacterClass::WHITESPACE));

    if (ch != ';')
    {
      break;
    }
    while ((ch = nextChar()) != '\n')
    {
      if (ch == EOF)
      {
        return Token::EOF_TOK;
      }
    }
  }
  bumpSpan();
  pushToToken(ch);
  switch (ch)
  {
    case '!': return Token::ATTRIBUTE_TOK;
    case '(': return Token::LPAREN_TOK;
    case ')': return Token::RPAREN_TOK;
    case '|':
      do
      {
        ch = nextChar();
        if (ch == EOF)
        {
          return Token::UNTERMINATED_QUOTED_SYMBOL;
        }
        pushToToken(ch);
      } while (ch != '|');
      return Token::QUOTED_SYMBOL;
    case '#':
      ch = nextChar();
      switch (ch)
      {
        case 'b':
          pushToToken(ch);
          // parse [01]+
          if (!parseNonEmptyCharList(CharacterClass::BIT))
          {
            parseError("Error expected bit string");
          }
          return Token::BINARY_LITERAL;
        case 'x':
          pushToToken(ch);
          // parse [0-9a-fA-F]+
          if (!parseNonEmptyCharList(CharacterClass::HEXADECIMAL_DIGIT))
          {
            parseError("Error expected hexidecimal string");
          }
          return Token::HEX_LITERAL;
        case 'f':
          pushToToken(ch);
          // parse [0-9]+m[0-9]+
          if (!parseNonEmptyCharList(CharacterClass::DECIMAL_DIGIT))
          {
            parseError("Error expected decimal for finite field value");
          }
          if (!parseLiteralChar('m'))
          {
            parseError("Error bad syntax for finite field value");
          }
          if (!parseNonEmptyCharList(CharacterClass::DECIMAL_DIGIT))
          {
            parseError("Error expected decimal for finite field size");
          }
          return Token::FIELD_LITERAL;
        default:
          // otherwise error
          parseError("Error finding token following #");
          break;
      }
      break;
    case '"':
      for (;;)
      {
        ch = nextChar();
        if (ch == EOF)
        {
          return Token::UNTERMINATED_STRING_LITERAL;
        }
        else if (ch == '"')
        {
          pushToToken(ch);
          ch = nextChar();
          // "" denotes the escape sequence for "
          if (ch != '"')
          {
            saveChar(ch);
            return Token::STRING_LITERAL;
          }
        }
        pushToToken(ch);
      }
      break;
    case ':':
      // parse a simple symbol
      if (!parseChar(CharacterClass::SYMBOL_START))
      {
        parseError("Error expected symbol following :");
      }
      parseNonEmptyCharList(CharacterClass::SYMBOL);
      return Token::KEYWORD;
    default:
      if (isCharacterClass(ch, CharacterClass::DECIMAL_DIGIT))
      {
        Token res = Token::INTEGER_LITERAL;
        // parse [0-9]*
        parseCharList(CharacterClass::DECIMAL_DIGIT);
        // maybe .[0-9]+
        ch = nextChar();
        if (ch == '.')
        {
          pushToToken(ch);
          res = Token::DECIMAL_LITERAL;
          // parse [0-9]+
          if (!parseNonEmptyCharList(CharacterClass::DECIMAL_DIGIT))
          {
            parseError("Error expected decimal string following .");
          }
        }
        else
        {
          // otherwise, undo
          saveChar(ch);
        }
        return res;
      }
      else if (isCharacterClass(ch, CharacterClass::SYMBOL_START))
      {
        // otherwise, we are a simple symbol or standard alphanumeric token
        // note that we group the case when `:` is here.
        parseCharList(CharacterClass::SYMBOL);
        // tokenize the current symbol, which may be a special case
        return tokenizeCurrentSymbol();
      }
      // otherwise error
      break;
  }
  parseError("Error finding token");
  return Token::NONE;
}

bool Lexer::parseLiteralChar(char chc)
{
  char ch = nextChar();
  if (ch != chc)
  {
    // will be an error
    return false;
  }
  pushToToken(ch);
  return true;
}

bool Lexer::parseChar(CharacterClass cc)
{
  char ch = nextChar();
  if (!isCharacterClass(ch, cc))
  {
    // will be an error
    return false;
  }
  pushToToken(ch);
  return true;
}

bool Lexer::parseNonEmptyCharList(CharacterClass cc)
{
  // must contain at least one character
  char ch = nextChar();
  if (!isCharacterClass(ch, cc))
  {
    // will be an error
    return false;
  }
  pushToToken(ch);
  parseCharList(cc);
  return true;
}

void Lexer::parseCharList(CharacterClass cc)
{
  char ch;
  for (;;)
  {
    ch = nextChar();
    if (!isCharacterClass(ch, cc))
    {
      // failed, we are done, put the character back
      saveChar(ch);
      return;
    }
    pushToToken(ch);
  }
}

Token Lexer::tokenizeCurrentSymbol() const
{
  //Assert(!d_token.empty());
  switch (d_token[0])
  {
    case 'a':
      if (d_token.size() == 2 && d_token[1] == 's')
      {
        return Token::AS_TOK;
      }
      break;
    case 'p':
      if (d_token.size() == 3 && d_token[1] == 'a' && d_token[2] == 'r')
      {
        return Token::PAR_TOK;
      }
      break;
    case 'l':
      if (d_token.size() == 3 && d_token[1] == 'e' && d_token[2] == 't')
      {
        return Token::LET_TOK;
      }
      break;
    case 'm':
      if (d_token.size() == 5 && d_token[1] == 'a' && d_token[2] == 't'
          && d_token[3] == 'c' && d_token[4] == 'h')
      {
        return Token::MATCH_TOK;
      }
      break;
    case '_':
      if (d_token.size() == 1)
      {
        return Token::INDEX_TOK;
      }
      break;
    default: break;
  }
  // otherwise not a special symbol
  return Token::SYMBOL;
}

}  // namespace atc