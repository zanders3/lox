#include "scanner.h"
#include "lox.h"
#include <cassert>

const char* tokentype_to_string(const TokenType token)
{
    switch (token)
    {
        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::MINUS: return "MINUS";
        case TokenType::PLUS: return "PLUS";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::SLASH: return "SLASH";
        case TokenType::STAR: return "STAR";
        case TokenType::BANG: return "BANG";
        case TokenType::BANG_EQUAL: return "BANG_EQUAL";
        case TokenType::EQUAL: return "EQUAL";
        case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::STRING: return "STRING";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::AND: return "AND";
        case TokenType::CLASS: return "CLASS";
        case TokenType::ELSE: return "ELSE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::FUN: return "FUN";
        case TokenType::FOR: return "FOR";
        case TokenType::IF: return "IF";
        case TokenType::NIL: return "NIL";
        case TokenType::OR: return "OR";
        case TokenType::PRINT: return "PRINT";
        case TokenType::RETURN: return "RETURN";
        case TokenType::SUPER: return "SUPER";
        case TokenType::THIS: return "THIS";
        case TokenType::TRUE: return "TRUE";
        case TokenType::VAR: return "VAR";
        case TokenType::WHILE: return "WHILE";
        case TokenType::END: return "END";
        default: return "???";
    }
}

struct Keyword
{
    const char* keyword;
    TokenType type;
};

Keyword g_keywords[] = {
    { "and",    TokenType::AND },
    { "class",  TokenType::CLASS },
    { "else",   TokenType::ELSE },
    { "false",  TokenType::FALSE },
    { "for",    TokenType::FOR },
    { "fun",    TokenType::FUN },
    { "if",     TokenType::IF },
    { "nil",    TokenType::NIL },
    { "or",     TokenType::OR },
    { "print",  TokenType::PRINT },
    { "return", TokenType::RETURN },
    { "super",  TokenType::SUPER },
    { "this",   TokenType::THIS },
    { "true",   TokenType::TRUE },
    { "var",    TokenType::VAR },
    { "while",  TokenType::WHILE }
};

struct Scanner
{
    const char* m_source;
    std::vector<Token>& m_tokens;
    int m_start, m_end, m_current, m_line, m_sourceLen;
    
    Scanner(const char* source, int sourceLen, std::vector<Token>& tokens)
    : m_source(source)
    , m_tokens(tokens)
    , m_start(0)
    , m_end(0)
    , m_current(0)
    , m_line(1)
    , m_sourceLen(sourceLen)
    {}
    
    inline bool IsAtEnd() { return m_current >= m_sourceLen; }
    
    char Advance()
    {
        return m_source[m_current++];
    }
    
    char Peek()
    {
        if (IsAtEnd()) return '\0';
        return m_source[m_current];
    }
    
    char PeekNext()
    {
        if (m_current + 1 >= m_sourceLen) return '\0';
        return m_source[m_current + 1];
    }
    
    bool Match(char expected)
    {
        if (IsAtEnd()) return false;
        if (m_source[m_current] != expected) return false;
        ++m_current;
        return true;
    }
    
    Token& AddToken(TokenType type)
    {
        m_tokens.push_back(Token());
        Token& tok = m_tokens[m_tokens.size() - 1];
        tok.type = type;
        int len = m_current - m_start;
        if (len >= 32) len = 31;
        assert(len > 0);
        for (int i = 0; i<len; i++)
            tok.lexeme[i] = m_source[m_start + i];
        tok.lexeme[len] = '\0';
        tok.line = m_line;
        return tok;
    }
    
    void ScanString()
    {
        while (Peek() != '"' && !IsAtEnd())
        {
            if (Peek() == '\n') ++m_line;
            Advance();
        }
        
        if (IsAtEnd())
        {
            lox_error(m_line, "Unterminated string");
            return;
        }
        
        Advance();
        AddToken(TokenType::STRING).stringLiteral = std::string(m_source + m_start + 1, m_current - m_start - 2);
    }
    
    inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }
    
    void Number()
    {
        while (IsDigit(Peek())) Advance();
        if (Peek() == '.' && IsDigit(PeekNext()))
        {
            Advance();//consume .
            while (IsDigit(Peek())) Advance();
        }
        
        AddToken(TokenType::NUMBER).numberLiteral = atoi(&m_source[m_start]);
    }
    
    inline bool IsLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
    inline bool IsLetterOrDigit(char c) { return IsDigit(c) || IsLetter(c); }
    
    void Identifier()
    {
        while (IsLetterOrDigit(Peek())) Advance();
        
        TokenType type = TokenType::IDENTIFIER;
        for (const Keyword& keyw : g_keywords)
        {
            const char* keyword = keyw.keyword;
            int i = 0;
            while (*keyword != '\0' && m_source[m_start + i] == *keyword && m_start + i < m_current)
            {
                ++keyword;
                ++i;
            }
            if (*keyword == '\0' && m_start + i == m_current)
            {
                type = keyw.type;
                break;
            }
        }
        Token& token = AddToken(type);
        if (type == TokenType::IDENTIFIER)
            token.stringLiteral = std::string(m_source + m_start, m_current - m_start);
    }
    
    void ScanTokens()
    {
        while (!IsAtEnd())
        {
            m_start = m_current;
            char c = Advance();
            switch (c)
            {
                case '(': AddToken(TokenType::LEFT_PAREN); break;
                case ')': AddToken(TokenType::RIGHT_PAREN); break;
                case '{': AddToken(TokenType::LEFT_BRACE); break;
                case '}': AddToken(TokenType::RIGHT_BRACE); break;
                case ',': AddToken(TokenType::COMMA); break;
                case '.': AddToken(TokenType::DOT); break;
                case '-': AddToken(TokenType::MINUS); break;
                case '+': AddToken(TokenType::PLUS); break;
                case ';': AddToken(TokenType::SEMICOLON); break;
                case '*': AddToken(TokenType::STAR); break;
                case '!': AddToken(Match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
                case '=': AddToken(Match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
                case '<': AddToken(Match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
                case '>': AddToken(Match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
                case '/':
                    if (Match('/'))
                    {
                        //Match comment
                        while (Peek() != '\n' && !IsAtEnd()) Advance();
                    }
                    else
                    {
                        AddToken(TokenType::SLASH);
                    }
                    break;
                case ' ':
                case '\r':
                case '\t'://Ignore whitespace
                    break;
                case '\n':
                    m_line++;
                    break;
                case '"': ScanString(); break;
                default:
                    if (IsDigit(c))
                        Number();
                    else if (IsLetter(c) || c == '_')
                        Identifier();
                    else
                        lox_error(m_line, "Unexpected character");
                    break;
            }
        }
        
        m_tokens.push_back(Token());
        Token& tok = m_tokens[m_tokens.size()-1];
        tok.type = TokenType::END;
        tok.lexeme[0] = '\0';
        tok.stringLiteral = std::string();
        tok.numberLiteral = 0;
        tok.line = m_line;
    }
};

void scanner_scan(const char* source, int sourceLen, std::vector<Token>& tokens)
{
    Scanner scanner(source, sourceLen, tokens);
    scanner.ScanTokens();
}
