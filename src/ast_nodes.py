class ASTNode:
    pass

class ProgramNode(ASTNode):
    def __init__(self, statements):
        self.statements = statements

class ImportStatementNode(ASTNode):
    def __init__(self, module_name, line, source=None, alias=None):
        self.module_name = module_name
        self.line = line
        self.source = source
        self.alias = alias

class AiraAiDirectiveNode(ASTNode):
    def __init__(self, prompt, line):
        self.prompt = prompt
        self.line = line

class SayStatementNode(ASTNode):
    def __init__(self, expressions, line):
        self.expressions = expressions
        self.line = line

class LetStatementNode(ASTNode):
    def __init__(self, name, expression, line):
        self.name = name
        self.expression = expression
        self.line = line

class AssignStatementNode(ASTNode):
    def __init__(self, name, expression, line):
        self.name = name
        self.expression = expression
        self.line = line

class IndexAssignStatementNode(ASTNode):
    def __init__(self, target, index, expression, line):
        self.target = target
        self.index = index
        self.expression = expression
        self.line = line

class MemberAssignStatementNode(ASTNode):
    def __init__(self, target, member, expression, line):
        self.target = target
        self.member = member
        self.expression = expression
        self.line = line

class IfStatementNode(ASTNode):
    def __init__(self, condition, then_branch, else_branch, line):
        self.condition = condition
        self.then_branch = then_branch
        self.else_branch = else_branch
        self.line = line

class WhileStatementNode(ASTNode):
    def __init__(self, condition, body, line):
        self.condition = condition
        self.body = body
        self.line = line

class ForInStatementNode(ASTNode):
    def __init__(self, var_name, iterable, body, line):
        self.var_name = var_name
        self.iterable = iterable
        self.body = body
        self.line = line

class TryCatchStatementNode(ASTNode):
    def __init__(self, try_body, error_var, catch_body, line):
        self.try_body = try_body
        self.error_var = error_var
        self.catch_body = catch_body
        self.line = line

class ThrowStatementNode(ASTNode):
    def __init__(self, expression, line):
        self.expression = expression
        self.line = line

class BreakStatementNode(ASTNode):
    def __init__(self, line):
        self.line = line

class ContinueStatementNode(ASTNode):
    def __init__(self, line):
        self.line = line

class ClassDefNode(ASTNode):
    def __init__(self, name, methods, line):
        self.name = name
        self.methods = methods
        self.line = line

class BlockNode(ASTNode):
    def __init__(self, statements):
        self.statements = statements

class FunctionDefNode(ASTNode):
    def __init__(self, name, params, body, line):
        self.name = name
        self.params = params
        self.body = body
        self.line = line

class ReturnStatementNode(ASTNode):
    def __init__(self, expression, line):
        self.expression = expression
        self.line = line

class ExpressionStatementNode(ASTNode):
    def __init__(self, expression):
        self.expression = expression

class BinaryOpNode(ASTNode):
    def __init__(self, left, op, right, line):
        self.left = left
        self.op = op
        self.right = right
        self.line = line

class UnaryOpNode(ASTNode):
    def __init__(self, op, right, line):
        self.op = op
        self.right = right
        self.line = line

class LiteralNode(ASTNode):
    def __init__(self, value, type_, line):
        self.value = value
        self.type = type_
        self.line = line

class IdentifierNode(ASTNode):
    def __init__(self, name, line):
        self.name = name
        self.line = line

class ThisNode(ASTNode):
    def __init__(self, line):
        self.line = line

class NewInstanceNode(ASTNode):
    def __init__(self, class_name, arguments, line):
        self.class_name = class_name
        self.arguments = arguments
        self.line = line

class FunctionCallNode(ASTNode):
    def __init__(self, callee, arguments, line):
        self.callee = callee
        self.arguments = arguments
        self.line = line

class MemberAccessNode(ASTNode):
    def __init__(self, target, member, line):
        self.target = target
        self.member = member
        self.line = line

class IndexAccessNode(ASTNode):
    def __init__(self, target, index, line):
        self.target = target
        self.index = index
        self.line = line

class ListNode(ASTNode):
    def __init__(self, elements, line):
        self.elements = elements
        self.line = line

class DictNode(ASTNode):
    def __init__(self, pairs, line):
        self.pairs = pairs
        self.line = line

class FunctionExprNode(ASTNode):
    def __init__(self, name, params, body, line):
        self.name = name
        self.params = params
        self.body = body
        self.line = line

class NamedArgNode(ASTNode):
    def __init__(self, name, value, line):
        self.name = name
        self.value = value
        self.line = line

class FStringNode(ASTNode):
    def __init__(self, parts, line):
        self.parts = parts
        self.line = line


