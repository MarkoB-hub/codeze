#include "tokenizer.h"
#include "my_string.h"
#include "container.h"
#include "debug.h"

#define INITIAL_TOKEN_CAPACITY 100

#define NUM_KEYWORDS 14
static char* gKeywords[] = {"case", "if", "else", "while", "switch", "continue",
	"break", "struct", "typedef", "return", "enum", "static",
	"const", "constexpr"};


#define NUM_TYPES 17
static char* gTypes[] = {"void", "i64", "i32", "i16", "i8", "u64",
	"u32", "u16", "u8", "sizet", "f32", "int", "float", "char", 
	"NULL", "double", "long"};

inline static i8
is_char_identifier(char c) {

	switch (c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '_':
      return 1;
    default:
      return 0;

	}
}


inline static i8
is_keyword(const char* word) {
	
  
	for (sizet i = 0; i < NUM_KEYWORDS; ++i) {

		if (str_equal(word, gKeywords[i])) return 1;
		
	}
	return 0;

}

inline static i8
is_type(const char* word) {
	
  
	for (sizet i = 0; i < NUM_TYPES; ++i) {

		if (str_equal(word, gTypes[i])) return 1;
		
	}
	return 0;

}

inline static i8
is_number(char c) {
	
	switch(c) {
		
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return 1;
    default:
      return 0;
	  
	}
}

inline static i8
in_quote(char c) {
	
	return c != '"';

}

Token*
tokens_make(String* text) {

	Token* tokens = array_create(INITIAL_TOKEN_CAPACITY, sizeof(Token));
	char* begin = text->data;
	char* pos = begin;

	while (*pos != '\0') {

		switch(*pos) {

		default:
			pos++;
			break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z': {
			Token token = {TOK_UNKNOWN, 0, pos - begin};
			static char word[256];
			while (is_char_identifier(*pos)) {

				word[token.length] = *pos;
				token.length++;
				pos++;
			}
			word[token.length] = '\0';

			if (is_keyword(word)) {
				
				token.type = TOK_KEYWORD;
			}
			else if (is_type(word)) {
				
				token.type = TOK_TYPE;
			}
			else{
				
				token.type = TOK_IDENTIFIER;
			}

			tokens = array_push(tokens, &token);
			break;
		}

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': {
			Token token = {TOK_NUMBER, 0, pos - begin};
			while (is_number(*pos)) {

				token.length++;
				pos++;
			}
			tokens = array_push(tokens, &token);
			break;
		}

		case '(': {
			Token token = {TOK_OPEN_PAREN, 1, pos - begin};
			pos++;
			tokens = array_push(tokens, &token);
			break;
		}

		case ')': {
			Token token = {TOK_CLOSED_PAREN, 1, pos - begin};
			pos++;
			tokens = array_push(tokens, &token);
			break;
		}

		case '{': {
			Token token = {TOK_OPEN_CURLY, 1, pos - begin};
			pos++;
			tokens = array_push(tokens, &token);
			break;
		}

		case '}': {
			Token token = {TOK_CLOSED_CURLY, 1, pos - begin};
			pos++;
			tokens = array_push(tokens, &token);
			break;
		}
		case '#': {
			Token token = {TOK_HASH, 1, pos - begin};
			pos++;
			tokens = array_push(tokens, &token);
			break;
		}
		case '"': {
			Token token = {TOK_STRING, 0, pos - begin};
			pos++;
			while (in_quote(*pos)) {
				pos++;
			}
			pos++;
			token.length = (pos - begin) - token.pos;
			tokens = array_push(tokens, &token);
			break;
		}
		case '<': {
			Token token = {TOK_STRING, 0, pos - begin};
			pos++;
			while ((*pos) != '>') {
				pos++;
			}
			pos++;
			token.length = (pos - begin) - token.pos;
			tokens = array_push(tokens, &token);
			break;
		}
		case ';': {
			Token token = {TOK_SEMICOLON, 1, pos - begin};
			pos++;
			tokens = array_push(tokens, &token);
			break;
		}
		case '/': {
			Token token = {TOK_COMMENT, 1, pos - begin};
			pos++;
			if (*pos == '/') {
				while (*pos != '\n' && *pos != '\0') {
					token.length++;
					pos++;
				}
			}
			else if (*pos == '*') {
				pos++;
				while (*pos != '\0' && (*pos != '*' && *(pos + 1) != '/')) {
					token.length++;
					pos++;
				}
			}
			tokens = array_push(tokens, &token);
			break;
		}

		}
	}

	return tokens;

}

static void
print_token(TokenType type) {

	printf("Token: ");
	
	switch(type) {
	case TOK_IDENTIFIER: printf("identifier \n"); break;
	case TOK_HASH: printf("hash \n"); break;
	case TOK_NUMBER: printf("number \n"); break;
	case TOK_OPEN_PAREN: printf("open paren \n"); break;
	case TOK_CLOSED_PAREN: printf("closed paren \n"); break;
	case TOK_OPEN_CURLY: printf("open curly \n"); break;
	case TOK_CLOSED_CURLY: printf("closed curly \n"); break;
	case TOK_OPEN_SQUARE: printf("open square \n"); break;
	case TOK_CLOSED_SQUARE: printf("closed square \n"); break;
	case TOK_KEYWORD: printf("keyword \n"); break;
	case TOK_STRING: printf("quote \n"); break;
	case TOK_UNKNOWN: printf("unknown\n"); break;

	}

}

void
print_tokens(Token* tokens) {

	for (sizet i = 0; i < ARRAY_LENGTH(tokens); ++i) {

		print_token(tokens[i].type);
		
	}
	
}
