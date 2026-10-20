import re

TOKEN_TYPES = [
    ("COMMENT",        r"(#|//).*"),
    ("WHITESPACE",     r"[ \t\r\n]+"),
    ("NUMBER",         r"\d+(\.\d+)?"),
    ("FSTRING",        r'f("([^"\\]|\\.)*"|\'([^\'\\]|\\.)*\')'),
    ("STRING",         r'("([^"\\]|\\.)*"|\'([^\'\\]|\\.)*\')'),
    ("KEYWORD",        r"\b(let|say|print|if|else|while|for|in|fn|return|import|from|load|use|true|false|null|and|or|not|try|catch|throw|class|new|this|break|continue)\b"),
    ("AT",             r"@"),
    ("IDENTIFIER",     r"[a-zA-Z_][a-zA-Z0-9_]*"),
    ("PLUS_ASSIGN",    r"\+="),
    ("MINUS_ASSIGN",   r"-="),
    ("MUL_ASSIGN",     r"\*="),
    ("DIV_ASSIGN",     r"/="),
    ("MOD_ASSIGN",     r"%="),
    ("EQ_EQ",          r"=="),
    ("NOT_EQ",         r"(!=|≠)"),
    ("LTE",            r"<="),
    ("GTE",            r">="),
    ("ASSIGN",         r"="),
    ("PLUS",           r"\+"),
    ("MINUS",          r"-"),
    ("MUL",            r"\*"),
    ("DIV",            r"/"),
    ("MOD",            r"%"),
    ("LT",             r"<"),
    ("GT",             r">"),
    ("NOT",            r"!"),
    ("LPAREN",         r"\("),
    ("RPAREN",         r"\)"),
    ("LBRACE",         r"\{"),
    ("RBRACE",         r"\}"),
    ("LBRACKET",       r"\["),
    ("RBRACKET",       r"\]"),
    ("COLON",          r":"),
    ("SEMICOLON",      r";"),
    ("COMMA",          r","),
    ("DOT",            r"\."),
]

class Token:
    def __init__(self, type_, value, line, col):
        self.type = type_
        self.value = value
        self.line = line
        self.col = col

    def __repr__(self):
        return f"Token({self.type}, {repr(self.value)}, line={self.line}, col={self.col})"

class Lexer:
    def __init__(self, code):
        self.code = code
        self.pos = 0
        self.line = 1
        self.col = 1
        self.tokens = []

    def tokenize(self):
        while self.pos < len(self.code):
            match = None
            for token_type, regex in TOKEN_TYPES:
                pattern = re.compile(regex)
                match = pattern.match(self.code, self.pos)
                if match:
                    val = match.group(0)
                    if token_type == "COMMENT":
                        pass
                    elif token_type == "WHITESPACE":
                        num_newlines = val.count('\n')
                        if num_newlines > 0:
                            self.line += num_newlines
                            self.col = len(val) - val.rfind('\n')
                        else:
                            self.col += len(val)
                    elif token_type == "FSTRING":
                        raw_str = val[2:-1].encode('utf-8').decode('unicode_escape')
                        self.tokens.append(Token("FSTRING", raw_str, self.line, self.col))
                        self.col += len(val)
                    elif token_type == "STRING":
                        raw_str = val[1:-1].encode('utf-8').decode('unicode_escape')
                        self.tokens.append(Token("STRING", raw_str, self.line, self.col))
                        self.col += len(val)
                    elif token_type == "NUMBER":
                        num_val = float(val) if '.' in val else int(val)
                        self.tokens.append(Token("NUMBER", num_val, self.line, self.col))
                        self.col += len(val)
                    elif token_type == "KEYWORD":
                        self.tokens.append(Token(val.upper(), val, self.line, self.col))
                        self.col += len(val)
                    else:
                        self.tokens.append(Token(token_type, val, self.line, self.col))
                        self.col += len(val)
                    self.pos = match.end()
                    break

            if not match:
                char = self.code[self.pos]
                raise SyntaxError(f"AiraLang Lexer Error: Unexpected character '{char}' at line {self.line}, col {self.col}")

        self.tokens.append(Token("EOF", None, self.line, self.col))
        return self.tokens
