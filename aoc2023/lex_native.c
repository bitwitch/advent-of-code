typedef struct {
	char *filepath;
	int line;
} SourcePos;

typedef enum {
	TOKEN_EOF,
	// 
	// reserve space for ascii literal characters
	//
	TOKEN_INT = 128,
	TOKEN_NAME,
	TOKEN_STR,
} TokenKind;

typedef struct {
	TokenKind kind;
	SourcePos pos;
    char *start, *end;
	union {
		U64 int_val;
        char *str_val;
        char *name;
	};
} Token;

static Token _token;
static char *_stream;
static int _current_line = 1;

void scan_str(void) {
    ++_stream; // skip opening quote
    char *str_start = _stream;
    while (*_stream != '"') {
		if (!isprint(*_stream)) {
			fprintf(stderr, "Invalid character found in string literal, '%c'", *_stream);
		}
		++_stream;
    }
    _token.kind = TOKEN_STR;
    _token.str_val = str_intern_range(str_start, _stream);
    ++_stream;
}

void next_token(void) {
repeat:
    _token.start = _stream;
	_token.pos.line = _current_line;
	switch (*_stream) {
        case ' ': case '\r': case '\t': case '\v': {
			while (isspace(*_stream)) {
				if (*_stream == '\n') break;
				++_stream;
			}
            goto repeat;
            break;
        }

        case '"': {
            scan_str();
            break;
		}

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': {
			_token.kind = TOKEN_INT;
			_token.int_val = strtoll(_stream, &_stream, 10);
			break;
		}

		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z': case 'A': case 'B': case 'C': case 'D':
		case 'E': case 'F': case 'G': case 'H': case 'I':
		case 'J': case 'K': case 'L': case 'M': case 'N':
		case 'O': case 'P': case 'Q': case 'R': case 'S':
		case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z': case '_': {
            while (isalnum(*_stream) || *_stream == '_')
                ++_stream;
            _token.kind = TOKEN_NAME;
            _token.name = str_intern_range(_token.start, _stream);
			break;
		}

		default: {
			if (*_stream == '\n') ++_current_line;
			_token.kind = *_stream;
			++_stream;
			break;
        }
	}
    _token.end = _stream;
}

void init_lexer(char *path, char *source) {
    _stream = source;
	_token.pos.filepath = str_intern(path);
	_current_line = 1;
    next_token();
}

static char *token_kind_to_str(TokenKind kind) {
	char str[64] = {0};
	switch (kind) {
    case TOKEN_INT:  sprintf(str, "integer"); break;
    case TOKEN_NAME: sprintf(str, "name");    break;
    case TOKEN_STR:  sprintf(str, "string");  break;
	default:
        if (kind < 128 && isprint(kind)) {
            sprintf(str, "%c", kind);
        } else {
            sprintf(str, "<ASCII %d>", kind);
        }
        break;
    }
    return str_intern(str);
}

static void expect_token(TokenKind kind) {
	if (_token.kind != kind) {
		fatal("Expected token %s, got %s", 
				token_kind_to_str(kind), 
				token_kind_to_str(_token.kind));
	}
	next_token();
}

static bool match_token(TokenKind kind) {
	if (_token.kind == kind) {
		next_token();
		return true;
	}
	return false;
}

static inline bool is_token(TokenKind kind) {
	return _token.kind == kind;
}

static inline bool is_token_name(char *name) {
	return _token.kind == TOKEN_NAME && _token.name == str_intern(name);
}

static bool match_token_name(char *name) {
	if (_token.kind == TOKEN_NAME && _token.name == str_intern(name)) {
		next_token();
		return true;
	}
	return false;
}


static U64 parse_int(void) {
	if (!is_token(TOKEN_INT)) {
		fatal("Expected integer, got %s", token_kind_to_str(_token.kind));
	}
	U64 number = _token.int_val;
	next_token();
	return number;
}

char *parse_name(void) {
	if (!is_token(TOKEN_NAME)) {
        fatal("Expected name, got %s", token_kind_to_str(_token.kind));
	}
	char *name = _token.name;
	next_token();
	return name;
}

