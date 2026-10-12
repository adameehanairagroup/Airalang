import sys
from ast_nodes import *

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
        self.missing_semicolon_lines = []

    def current(self):
        return self.tokens[self.pos]

    def peek(self, offset=1):
        if self.pos + offset < len(self.tokens):
            return self.tokens[self.pos + offset]
        return self.tokens[-1]

    def match(self, *types):
        if self.current().type in types:
            tok = self.current()
            self.pos += 1
            return tok
        return None

    def expect(self, type_):
        tok = self.match(type_)
        if not tok:
            curr = self.current()
            raise SyntaxError(f"AiraLang SyntaxError: Expected '{type_}' but got '{curr.type}' ({repr(curr.value)}) at line {curr.line}, col {curr.col}")
        return tok

    def consume_semicolon(self, line=None):
        if self.match("SEMICOLON"):
            return
        if line is None:
            line = self.tokens[self.pos - 1].line if self.pos > 0 else self.current().line
        self.missing_semicolon_lines.append(line)

    def parse_block(self):
        self.expect("LBRACE")
        stmts = []
        while self.current().type not in ("RBRACE", "EOF"):
            s = self.parse_statement()
            if s:
                stmts.append(s)
        self.expect("RBRACE")
        return BlockNode(stmts)

    def parse(self):
        statements = []
        while self.current().type != "EOF":
            stmt = self.parse_statement()
            if stmt:
                statements.append(stmt)

        if self.missing_semicolon_lines and not getattr(self, "suppress_semicolon_notice", False):
            unique_lines = sorted(set(self.missing_semicolon_lines))
            lines_str = ", ".join(str(l) for l in unique_lines)

            CYAN = "\033[1;36m"
            YELLOW = "\033[1;33m"
            MAGENTA = "\033[1;35m"
            RESET = "\033[0m"
            BOLD = "\033[1m"

            ai_tag = ""
            try:
                from stdlib_modules import _GLOBAL_AI_STATE
                has_key = any(v for k, v in _GLOBAL_AI_STATE["keys"].items() if k != "ollama" and v)
                if has_key:
                    active_p = _GLOBAL_AI_STATE.get("active_provider", "ai").upper()
                    ai_tag = f" [{active_p}]"
            except Exception:
                pass

            single_notice = (
                f"\n{CYAN}✨ [Aira Auto-Fix{ai_tag}]:{RESET} {YELLOW}Hey there! Semicolon (;) missed at line {lines_str}.{RESET} "
                f"I've put it automatically into your code — relax & enjoy coding! {MAGENTA}💻✨🚀{RESET}\n"
            )
            sys.stderr.write(single_notice)
            sys.stderr.flush()

        return ProgramNode(statements)

    def parse_statement(self):
        tok = self.current()

        if self.match("SEMICOLON"):
            return None

        # Standalone block statement: `{ stmt1; stmt2; }`
        if tok.type == "LBRACE":
            return self.parse_block_or_statement()

        # import / load / use statement: `import os;`, `import "math";`, `import "helper.aira";`
        if tok.type in ("IMPORT", "LOAD", "USE"):
            self.pos += 1
            if self.current().type in ("STRING", "IDENTIFIER"):
                mod_tok = self.current()
                self.pos += 1
            else:
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Expected module name or string after '{tok.value}' at line {curr.line}, col {curr.col}")
            self.consume_semicolon(tok.line)
            return ImportStatementNode(str(mod_tok.value), tok.line)

        # say / print statement: `say "A", "B", 10;`
        if tok.type in ("SAY", "PRINT"):
            self.pos += 1
            expressions = [self.parse_expression()]
            while self.match("COMMA"):
                expressions.append(self.parse_expression())
            self.consume_semicolon(tok.line)
            return SayStatementNode(expressions, tok.line)

        # let statement: `let name = expr;`
        if tok.type == "LET":
            self.pos += 1
            name_tok = self.expect("IDENTIFIER")
            self.expect("ASSIGN")
            expr = self.parse_expression()
            self.consume_semicolon(tok.line)
            return LetStatementNode(name_tok.value, expr, tok.line)

        # if statement: `if (cond) { ... } else if (cond2) { ... } else { ... }`
        if tok.type == "IF":
            self.pos += 1
            has_paren = self.match("LPAREN")
            cond = self.parse_expression()
            if has_paren:
                self.expect("RPAREN")
            if self.current().type != "LBRACE":
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for if-statement at line {tok.line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
            then_branch = self.parse_block()
            else_branch = None
            if self.match("ELSE"):
                if self.current().type == "IF":
                    else_branch = self.parse_statement()
                else:
                    if self.current().type != "LBRACE":
                        curr = self.current()
                        raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for else-block at line {self.tokens[self.pos - 1].line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
                    else_branch = self.parse_block()
            return IfStatementNode(cond, then_branch, else_branch, tok.line)

        # while statement: `while (cond) { ... }`
        if tok.type == "WHILE":
            self.pos += 1
            has_paren = self.match("LPAREN")
            cond = self.parse_expression()
            if has_paren:
                self.expect("RPAREN")
            if self.current().type != "LBRACE":
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for while loop at line {tok.line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
            body = self.parse_block()
            return WhileStatementNode(cond, body, tok.line)

        # for statement: `for item in items { ... }`
        if tok.type == "FOR":
            self.pos += 1
            has_paren = self.match("LPAREN")
            self.match("LET") # optional let
            var_tok = self.expect("IDENTIFIER")
            self.expect("IN")
            iterable = self.parse_expression()
            if has_paren:
                self.expect("RPAREN")
            if self.current().type != "LBRACE":
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for for-loop at line {tok.line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
            body = self.parse_block()
            return ForInStatementNode(var_tok.value, iterable, body, tok.line)

        # try / catch statement: `try { ... } catch (err) { ... }`
        if tok.type == "TRY":
            self.pos += 1
            if self.current().type != "LBRACE":
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for try block at line {tok.line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
            try_body = self.parse_block()
            self.expect("CATCH")
            has_paren = self.match("LPAREN")
            err_var = "error"
            if self.current().type == "IDENTIFIER":
                err_var = self.expect("IDENTIFIER").value
            if has_paren:
                self.expect("RPAREN")
            if self.current().type != "LBRACE":
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for catch block at line {tok.line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
            catch_body = self.parse_block()
            return TryCatchStatementNode(try_body, err_var, catch_body, tok.line)

        # throw statement: `throw "Error description";`
        if tok.type == "THROW":
            self.pos += 1
            expr = self.parse_expression()
            self.consume_semicolon(tok.line)
            return ThrowStatementNode(expr, tok.line)

        # break statement: `break;`
        if tok.type == "BREAK":
            self.pos += 1
            self.consume_semicolon(tok.line)
            return BreakStatementNode(tok.line)

        # continue statement: `continue;`
        if tok.type == "CONTINUE":
            self.pos += 1
            self.consume_semicolon(tok.line)
            return ContinueStatementNode(tok.line)

        # fn statement: `fn name(a, b) { ... }`
        if tok.type == "FN":
            self.pos += 1
            name_tok = self.expect("IDENTIFIER")
            self.expect("LPAREN")
            params = []
            if self.current().type != "RPAREN":
                params.append(self.expect("IDENTIFIER").value)
                while self.match("COMMA"):
                    params.append(self.expect("IDENTIFIER").value)
            self.expect("RPAREN")
            if self.current().type != "LBRACE":
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for function '{name_tok.value}' at line {tok.line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
            body = self.parse_block()
            return FunctionDefNode(name_tok.value, params, body, tok.line)

        # class statement: `class Person { fn init(name) { ... } fn greet() { ... } }`
        if tok.type == "CLASS":
            self.pos += 1
            name_tok = self.expect("IDENTIFIER")
            if self.current().type != "LBRACE":
                curr = self.current()
                raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for class '{name_tok.value}' at line {tok.line}. Block braces '{{}}' are mandatory in AiraLang! Found '{curr.value or curr.type}' instead.")
            self.expect("LBRACE")
            methods = {}
            while self.current().type != "RBRACE" and self.current().type != "EOF":
                if self.match("FN") or self.current().type == "IDENTIFIER":
                    if self.tokens[self.pos - 1].type != "FN":
                        m_name_tok = self.expect("IDENTIFIER")
                    else:
                        m_name_tok = self.expect("IDENTIFIER")
                    self.expect("LPAREN")
                    m_params = []
                    if self.current().type != "RPAREN":
                        m_params.append(self.expect("IDENTIFIER").value)
                        while self.match("COMMA"):
                            m_params.append(self.expect("IDENTIFIER").value)
                    self.expect("RPAREN")
                    if self.current().type != "LBRACE":
                        curr = self.current()
                        raise SyntaxError(f"AiraLang SyntaxError: Missing block brace '{{' for method '{m_name_tok.value}' in class '{name_tok.value}' at line {m_name_tok.line}. Block braces '{{}}' are mandatory in AiraLang!")
                    m_body = self.parse_block()
                    methods[m_name_tok.value] = FunctionDefNode(m_name_tok.value, m_params, m_body, m_name_tok.line)
                elif self.match("SEMICOLON"):
                    pass
                else:
                    curr = self.current()
                    raise SyntaxError(f"AiraLang SyntaxError: Expected method in class '{name_tok.value}' at line {curr.line}")
            self.expect("RBRACE")
            return ClassDefNode(name_tok.value, methods, tok.line)

        # return statement: `return expr;`
        if tok.type == "RETURN":
            self.pos += 1
            expr = None
            if self.current().type not in ("SEMICOLON", "RBRACE", "EOF"):
                expr = self.parse_expression()
            self.consume_semicolon(tok.line)
            return ReturnStatementNode(expr, tok.line)

        # expression / assignment / shorthand assignment statement
        expr = self.parse_expression()
        assign_op = self.match("ASSIGN", "PLUS_ASSIGN", "MINUS_ASSIGN", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN")
        if assign_op:
            val_expr = self.parse_expression()
            self.consume_semicolon(tok.line)

            if assign_op.type != "ASSIGN":
                bin_op_symbol = {
                    "PLUS_ASSIGN": "+",
                    "MINUS_ASSIGN": "-",
                    "MUL_ASSIGN": "*",
                    "DIV_ASSIGN": "/",
                    "MOD_ASSIGN": "%"
                }[assign_op.type]
                val_expr = BinaryOpNode(expr, bin_op_symbol, val_expr, assign_op.line)

            if isinstance(expr, IdentifierNode):
                return AssignStatementNode(expr.name, val_expr, tok.line)
            elif isinstance(expr, IndexAccessNode):
                return IndexAssignStatementNode(expr.target, expr.index, val_expr, tok.line)
            elif isinstance(expr, MemberAccessNode):
                return MemberAssignStatementNode(expr.target, expr.member, val_expr, tok.line)
            else:
                raise SyntaxError(f"AiraLang SyntaxError: Invalid assignment target at line {tok.line}")

        self.consume_semicolon(tok.line)
        return ExpressionStatementNode(expr)

    def parse_block_or_statement(self):
        if self.match("LBRACE"):
            stmts = []
            while self.current().type not in ("RBRACE", "EOF"):
                s = self.parse_statement()
                if s:
                    stmts.append(s)
            self.expect("RBRACE")
            return BlockNode(stmts)
        else:
            return self.parse_statement()

    # Expressions
    def parse_expression(self):
        return self.parse_logical_or()

    def parse_logical_or(self):
        left = self.parse_logical_and()
        while self.current().type == "OR":
            op = self.current()
            self.pos += 1
            right = self.parse_logical_and()
            left = BinaryOpNode(left, "or", right, op.line)
        return left

    def parse_logical_and(self):
        left = self.parse_equality()
        while self.current().type == "AND":
            op = self.current()
            self.pos += 1
            right = self.parse_equality()
            left = BinaryOpNode(left, "and", right, op.line)
        return left

    def parse_equality(self):
        left = self.parse_relational()
        while self.current().type in ("EQ_EQ", "NOT_EQ"):
            op = self.current()
            self.pos += 1
            right = self.parse_relational()
            left = BinaryOpNode(left, op.value, right, op.line)
        return left

    def parse_relational(self):
        left = self.parse_additive()
        while self.current().type in ("LT", "GT", "LTE", "GTE"):
            op = self.current()
            self.pos += 1
            right = self.parse_additive()
            left = BinaryOpNode(left, op.value, right, op.line)
        return left

    def parse_additive(self):
        left = self.parse_multiplicative()
        while self.current().type in ("PLUS", "MINUS"):
            op = self.current()
            self.pos += 1
            right = self.parse_multiplicative()
            left = BinaryOpNode(left, op.value, right, op.line)
        return left

    def parse_multiplicative(self):
        left = self.parse_unary()
        while self.current().type in ("MUL", "DIV", "MOD"):
            op = self.current()
            self.pos += 1
            right = self.parse_unary()
            left = BinaryOpNode(left, op.value, right, op.line)
        return left

    def parse_unary(self):
        if self.current().type in ("MINUS", "NOT"):
            op = self.current()
            self.pos += 1
            right = self.parse_unary()
            return UnaryOpNode(op.value, right, op.line)
        return self.parse_postfix()

    def parse_call_arg(self):
        if self.current().type == "IDENTIFIER" and self.peek().type == "ASSIGN":
            name_tok = self.expect("IDENTIFIER")
            self.expect("ASSIGN")
            val_expr = self.parse_expression()
            return NamedArgNode(name_tok.value, val_expr, name_tok.line)
        return self.parse_expression()

    def parse_postfix(self):
        atom = self.parse_atom()
        while True:
            if self.match("LPAREN"):
                args = []
                if self.current().type != "RPAREN":
                    args.append(self.parse_call_arg())
                    while self.match("COMMA"):
                        args.append(self.parse_call_arg())
                self.expect("RPAREN")
                atom = FunctionCallNode(atom, args, getattr(atom, 'line', self.current().line))
            elif self.match("DOT"):
                member_tok = self.expect("IDENTIFIER")
                atom = MemberAccessNode(atom, member_tok.value, member_tok.line)
            elif self.match("LBRACKET"):
                idx_expr = self.parse_expression()
                self.expect("RBRACKET")
                atom = IndexAccessNode(atom, idx_expr, getattr(atom, 'line', self.current().line))
            else:
                break
        return atom

    def parse_fstring(self, tok):
        from lexer import Lexer
        raw_str = tok.value
        line = tok.line
        parts = []
        i = 0
        n = len(raw_str)
        cur_text = []
        while i < n:
            if raw_str[i] == '{':
                if i + 1 < n and raw_str[i+1] == '{':
                    cur_text.append('{')
                    i += 2
                    continue
                if cur_text:
                    parts.append((False, "".join(cur_text)))
                    cur_text = []
                i += 1
                start = i
                brace_count = 1
                while i < n and brace_count > 0:
                    if raw_str[i] == '{':
                        brace_count += 1
                    elif raw_str[i] == '}':
                        brace_count -= 1
                    i += 1
                if brace_count > 0:
                    raise SyntaxError(f"AiraLang SyntaxError: Unclosed '{{' in f-string at line {line}")
                expr_str = raw_str[start:i-1].strip()
                if expr_str:
                    sub_tokens = Lexer(expr_str).tokenize()
                    sub_parser = Parser(sub_tokens)
                    expr_ast = sub_parser.parse_expression()
                    parts.append((True, expr_ast))
            elif raw_str[i] == '}':
                if i + 1 < n and raw_str[i+1] == '}':
                    cur_text.append('}')
                    i += 2
                    continue
                cur_text.append('}')
                i += 1
            else:
                cur_text.append(raw_str[i])
                i += 1
        if cur_text:
            parts.append((False, "".join(cur_text)))
        return FStringNode(parts, line)

    def parse_atom(self):
        tok = self.current()
        if self.match("NUMBER"):
            return LiteralNode(tok.value, "NUMBER", tok.line)
        if self.match("STRING"):
            return LiteralNode(tok.value, "STRING", tok.line)
        if self.match("FSTRING"):
            return self.parse_fstring(tok)

        if self.match("TRUE"):
            return LiteralNode(True, "BOOLEAN", tok.line)
        if self.match("FALSE"):
            return LiteralNode(False, "BOOLEAN", tok.line)
        if self.match("NULL"):
            return LiteralNode(None, "NULL", tok.line)
        if self.match("THIS"):
            return ThisNode(tok.line)
        if self.match("NEW"):
            class_tok = self.expect("IDENTIFIER")
            args = []
            if self.match("LPAREN"):
                if self.current().type != "RPAREN":
                    args.append(self.parse_call_arg())
                    while self.match("COMMA"):
                        args.append(self.parse_call_arg())
                self.expect("RPAREN")
            return NewInstanceNode(class_tok.value, args, tok.line)
        if self.match("IDENTIFIER"):
            return IdentifierNode(tok.value, tok.line)
        if self.match("LPAREN"):
            expr = self.parse_expression()
            self.expect("RPAREN")
            return expr
        if self.match("LBRACKET"):
            elements = []
            if self.current().type != "RBRACKET":
                elements.append(self.parse_expression())
                while self.match("COMMA"):
                    elements.append(self.parse_expression())
            self.expect("RBRACKET")
            return ListNode(elements, tok.line)
        if self.match("LBRACE"):
            pairs = []
            if self.current().type != "RBRACE":
                key_expr = self.parse_expression()
                self.expect("COLON")
                val_expr = self.parse_expression()
                pairs.append((key_expr, val_expr))
                while self.match("COMMA"):
                    if self.current().type == "RBRACE":
                        break
                    k = self.parse_expression()
                    self.expect("COLON")
                    v = self.parse_expression()
                    pairs.append((k, v))
            self.expect("RBRACE")
            return DictNode(pairs, tok.line)
        if self.match("FN"):
            fn_name = None
            if self.current().type == "IDENTIFIER":
                fn_name = self.expect("IDENTIFIER").value
            self.expect("LPAREN")
            params = []
            if self.current().type != "RPAREN":
                params.append(self.expect("IDENTIFIER").value)
                while self.match("COMMA"):
                    params.append(self.expect("IDENTIFIER").value)
            self.expect("RPAREN")
            body = self.parse_block_or_statement()
            return FunctionExprNode(fn_name, params, body, tok.line)

        raise SyntaxError(f"AiraLang SyntaxError: Unexpected token '{tok.type}' ({repr(tok.value)}) at line {tok.line}, col {tok.col}")
