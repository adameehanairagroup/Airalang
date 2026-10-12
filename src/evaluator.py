import math
import time
import os
import sys
import json
from stdlib_modules import BUILTIN_MODULES, AiraModule

class ReturnException(Exception):
    def __init__(self, value):
        self.value = value

class BreakException(Exception):
    pass

class ContinueException(Exception):
    pass

class AiraRuntimeException(Exception):
    pass

class Environment:
    def __init__(self, parent=None):
        self.values = {}
        self.parent = parent

    def get(self, name, line=1):
        if name in self.values:
            return self.values[name]
        if self.parent:
            return self.parent.get(name, line)
        raise NameError(f"AiraLang RuntimeError: Variable '{name}' is not defined (Line {line})")

    def set(self, name, value):
        self.values[name] = value

    def assign(self, name, value, line=1):
        if name in self.values:
            self.values[name] = value
            return
        if self.parent:
            self.parent.assign(name, value, line)
            return
        raise NameError(f"AiraLang RuntimeError: Cannot assign to undefined variable '{name}' (Line {line})")

class AiraFunction:
    def __init__(self, def_node, closure_env):
        self.def_node = def_node
        self.closure_env = closure_env

    def __call__(self, *args):
        return self.call(Evaluator._current_evaluator, list(args))

    def call(self, evaluator, args, kwargs=None, this_instance=None):
        fn_env = Environment(parent=self.closure_env)
        if this_instance is not None:
            fn_env.set("this", this_instance)
        for param, arg in zip(self.def_node.params, args):
            fn_env.set(param, arg)
        if kwargs:
            for k, v in kwargs.items():
                fn_env.set(k, v)
        try:
            evaluator.eval_node(self.def_node.body, fn_env)
        except ReturnException as ret:
            return ret.value
        return None

class AiraClass:
    def __init__(self, name, methods, closure_env):
        self.name = name
        self.methods = methods
        self.closure_env = closure_env

    def instantiate(self, evaluator, args, kwargs=None):
        instance = AiraInstance(self)
        if "init" in self.methods:
            init_fn = AiraFunction(self.methods["init"], self.closure_env)
            init_fn.call(evaluator, args, kwargs=kwargs, this_instance=instance)
        elif "__init__" in self.methods:
            init_fn = AiraFunction(self.methods["__init__"], self.closure_env)
            init_fn.call(evaluator, args, kwargs=kwargs, this_instance=instance)
        return instance

    def __call__(self, *args):
        raise TypeError(f"AiraClass '{self.name}' must be instantiated with 'new {self.name}(...)'")

class AiraInstance:
    def __init__(self, class_ref):
        self.class_ref = class_ref
        self.fields = {}

    def get(self, name, line=1):
        if name in self.fields:
            return self.fields[name]
        if name in self.class_ref.methods:
            fn_node = self.class_ref.methods[name]
            bound_fn = AiraFunction(fn_node, self.class_ref.closure_env)
            return lambda *args: bound_fn.call(Evaluator._current_evaluator, args, this_instance=self)
        raise AttributeError(f"AiraLang AttributeError: Instance of '{self.class_ref.name}' has no property or method '{name}' (Line {line})")

    def set(self, name, value):
        self.fields[name] = value

    def __repr__(self):
        return f"<Instance of {self.class_ref.name} fields={self.fields}>"

class Evaluator:
    _current_evaluator = None

    def __init__(self):
        Evaluator._current_evaluator = self
        self.global_env = Environment()
        self.setup_builtins()

    def setup_builtins(self):
        self.global_env.set("print", lambda *args: print(*args))
        self.global_env.set("say", lambda *args: print(*args))
        self.global_env.set("len", lambda x: len(x))
        self.global_env.set("type", lambda x: type(x).__name__)
        self.global_env.set("str", lambda x: str(x))
        self.global_env.set("num", lambda x: float(x) if '.' in str(x) else int(x))
        self.global_env.set("input", lambda prompt="": input(prompt))
        self.global_env.set("range", lambda start, stop=None, step=1: list(range(int(start), int(stop), int(step))) if stop is not None else list(range(int(start))))
        self.global_env.set("min", lambda *args: min(args[0]) if len(args) == 1 and isinstance(args[0], (list, tuple)) else min(*args))
        self.global_env.set("max", lambda *args: max(args[0]) if len(args) == 1 and isinstance(args[0], (list, tuple)) else max(*args))
        self.global_env.set("abs", lambda x: abs(x))
        self.global_env.set("sum", lambda lst: sum(lst) if isinstance(lst, (list, tuple)) else lst)
        self.global_env.set("append", lambda lst, item: lst.append(item) or lst)
        self.global_env.set("pop", lambda lst, idx=-1: lst.pop(idx))
        self.global_env.set("split", lambda s, sep=" ": str(s).split(sep))
        self.global_env.set("join", lambda lst, sep=" ": sep.join(str(x) for x in lst))
        self.global_env.set("keys", lambda d: list(d.keys()) if isinstance(d, dict) else [])
        self.global_env.set("values", lambda d: list(d.values()) if isinstance(d, dict) else [])
        self.global_env.set("hex", lambda x: hex(int(x)))
        self.global_env.set("bin", lambda x: bin(int(x)))
        self.global_env.set("ord", lambda c: ord(str(c)[0]))
        self.global_env.set("chr", lambda x: chr(int(x)))
        self.global_env.set("sorted", lambda lst, reverse=False: sorted(lst, reverse=bool(reverse)))
        self.global_env.set("reversed", lambda lst: list(reversed(lst)))
        self.global_env.set("enumerate", lambda lst: [[i, v] for i, v in enumerate(lst)])
        self.global_env.set("zip", lambda *lsts: [list(items) for items in zip(*lsts)])

    def eval(self, ast):
        return self.eval_node(ast, self.global_env)

    def eval_node(self, node, env):
        if node is None:
            return None
        Evaluator._current_evaluator = self
        nodetype = node.__class__.__name__

        if nodetype == "ProgramNode":
            res = None
            for stmt in node.statements:
                res = self.eval_node(stmt, env)
            return res

        elif nodetype == "ImportStatementNode":
            mod_name = node.module_name
            if mod_name in BUILTIN_MODULES:
                module_instance = BUILTIN_MODULES[mod_name]()
                env.set(mod_name, module_instance)
                return module_instance
            
            # File import check (.aira or .py)
            target_path = mod_name
            if not os.path.exists(target_path) and not target_path.endswith(".aira"):
                target_path = mod_name + ".aira"

            if os.path.exists(target_path):
                with open(target_path, "r", encoding="utf-8") as f:
                    code = f.read()
                from lexer import Lexer
                from parser import Parser
                sub_tokens = Lexer(code).tokenize()
                sub_ast = Parser(sub_tokens).parse()
                mod_env = Environment(parent=self.global_env)
                self.eval_node(sub_ast, mod_env)
                base_name = os.path.splitext(os.path.basename(target_path))[0]
                aira_mod = AiraModule(base_name, mod_env.values)
                env.set(base_name, aira_mod)
                return aira_mod
            else:
                raise ImportError(f"AiraLang ImportError: Module or file '{mod_name}' not found (Line {node.line})")

        elif nodetype == "SayStatementNode":
            values = [self.eval_node(expr, env) for expr in node.expressions]
            print(*values)
            return values[-1] if values else None

        elif nodetype == "LetStatementNode":
            val = self.eval_node(node.expression, env)
            env.set(node.name, val)
            return val

        elif nodetype == "AssignStatementNode":
            val = self.eval_node(node.expression, env)
            env.assign(node.name, val, node.line)
            return val

        elif nodetype == "IndexAssignStatementNode":
            target = self.eval_node(node.target, env)
            index = self.eval_node(node.index, env)
            val = self.eval_node(node.expression, env)
            if isinstance(target, list):
                target[int(index)] = val
                return val
            elif isinstance(target, dict):
                target[index] = val
                return val
            else:
                raise TypeError(f"AiraLang RuntimeError: Cannot index-assign to type '{type(target).__name__}' (Line {node.line})")

        elif nodetype == "MemberAssignStatementNode":
            target = self.eval_node(node.target, env)
            val = self.eval_node(node.expression, env)
            if isinstance(target, AiraInstance):
                target.set(node.member, val)
                return val
            elif isinstance(target, dict):
                target[node.member] = val
                return val
            else:
                setattr(target, node.member, val)
                return val

        elif nodetype == "IfStatementNode":
            cond = self.eval_node(node.condition, env)
            if cond:
                return self.eval_node(node.then_branch, env)
            elif node.else_branch:
                return self.eval_node(node.else_branch, env)
            return None

        elif nodetype == "WhileStatementNode":
            res = None
            while self.eval_node(node.condition, env):
                try:
                    res = self.eval_node(node.body, env)
                except BreakException:
                    break
                except ContinueException:
                    continue
            return res

        elif nodetype == "ForInStatementNode":
            iterable = self.eval_node(node.iterable, env)
            res = None
            loop_items = []
            if isinstance(iterable, (list, tuple, set)):
                loop_items = list(iterable)
            elif isinstance(iterable, str):
                loop_items = list(iterable)
            elif isinstance(iterable, dict):
                loop_items = list(iterable.keys())
            elif hasattr(iterable, '__iter__'):
                loop_items = list(iterable)
            else:
                raise TypeError(f"AiraLang TypeError: '{type(iterable).__name__}' is not iterable (Line {node.line})")

            for item in loop_items:
                loop_env = Environment(parent=env)
                loop_env.set(node.var_name, item)
                try:
                    res = self.eval_node(node.body, loop_env)
                except BreakException:
                    break
                except ContinueException:
                    continue
            return res

        elif nodetype == "TryCatchStatementNode":
            try:
                return self.eval_node(node.try_body, env)
            except (ReturnException, BreakException, ContinueException):
                raise
            except Exception as e:
                catch_env = Environment(parent=env)
                err_msg = str(e)
                if err_msg.startswith("AiraLang RuntimeError: "):
                    err_msg = err_msg.replace("AiraLang RuntimeError: ", "")
                catch_env.set(node.error_var, err_msg)
                return self.eval_node(node.catch_body, catch_env)

        elif nodetype == "ThrowStatementNode":
            val = self.eval_node(node.expression, env)
            raise AiraRuntimeException(str(val))

        elif nodetype == "BreakStatementNode":
            raise BreakException()

        elif nodetype == "ContinueStatementNode":
            raise ContinueException()

        elif nodetype == "ClassDefNode":
            cls = AiraClass(node.name, node.methods, env)
            env.set(node.name, cls)
            return cls

        elif nodetype == "NewInstanceNode":
            cls = env.get(node.class_name, node.line)
            args = []
            kwargs = {}
            for arg in node.arguments:
                if getattr(arg, "__class__", None).__name__ == "NamedArgNode":
                    kwargs[arg.name] = self.eval_node(arg.value, env)
                else:
                    args.append(self.eval_node(arg, env))
            if isinstance(cls, AiraClass):
                return cls.instantiate(self, args, kwargs=kwargs)
            raise TypeError(f"AiraLang TypeError: '{node.class_name}' is not a class (Line {node.line})")

        elif nodetype == "ThisNode":
            return env.get("this", node.line)

        elif nodetype == "BlockNode":
            block_env = Environment(parent=env)
            res = None
            for stmt in node.statements:
                res = self.eval_node(stmt, block_env)
            return res

        elif nodetype == "FunctionDefNode":
            fn = AiraFunction(node, env)
            env.set(node.name, fn)
            return fn

        elif nodetype == "FunctionExprNode":
            fn = AiraFunction(node, env)
            if node.name:
                env.set(node.name, fn)
            return fn

        elif nodetype == "ReturnStatementNode":
            val = self.eval_node(node.expression, env) if node.expression else None
            raise ReturnException(val)

        elif nodetype == "ExpressionStatementNode":
            return self.eval_node(node.expression, env)

        elif nodetype == "LiteralNode":
            return node.value

        elif nodetype == "FStringNode":
            result_pieces = []
            for is_expr, item in node.parts:
                if is_expr:
                    val = self.eval_node(item, env)
                    result_pieces.append(str(val) if val is not None else "null")
                else:
                    result_pieces.append(item)
            return "".join(result_pieces)


        elif nodetype == "IdentifierNode":
            return env.get(node.name, node.line)

        elif nodetype == "ListNode":
            return [self.eval_node(el, env) for el in node.elements]

        elif nodetype == "DictNode":
            d = {}
            for k_expr, v_expr in node.pairs:
                k = self.eval_node(k_expr, env)
                v = self.eval_node(v_expr, env)
                d[k] = v
            return d

        elif nodetype == "MemberAccessNode":
            target = self.eval_node(node.target, env)
            member = node.member

            # Class instance access
            if isinstance(target, AiraInstance):
                return target.get(member, node.line)

            # Module access
            elif isinstance(target, AiraModule):
                return target.get(member)

            # Dictionary access
            elif isinstance(target, dict):
                if member in target:
                    return target[member]
                if member == "keys": return lambda: list(target.keys())
                if member == "values": return lambda: list(target.values())
                if member == "items": return lambda: [[k, v] for k, v in target.items()]
                if member == "get": return lambda k, default=None: target.get(k, default)
                if member == "has" or member == "contains": return lambda k: k in target
                if member == "length": return len(target)

            # String methods & properties
            elif isinstance(target, str):
                if member == "length": return len(target)
                if member == "upper": return lambda: target.upper()
                if member == "lower": return lambda: target.lower()
                if member == "title": return lambda: target.title()
                if member == "split": return lambda sep=" ": target.split(sep)
                if member == "trim" or member == "strip": return lambda: target.strip()
                if member == "replace": return lambda old, new: target.replace(str(old), str(new))
                if member == "contains": return lambda sub: str(sub) in target
                if member == "starts_with" or member == "startswith": return lambda sub: target.startswith(str(sub))
                if member == "ends_with" or member == "endswith": return lambda sub: target.endswith(str(sub))
                if member == "slice": return lambda start, end=None: target[int(start):int(end) if end is not None else None]
                if member == "reverse": return lambda: target[::-1]
                if member == "count": return lambda sub: target.count(str(sub))

            # List methods & properties
            elif isinstance(target, list):
                if member == "length": return len(target)
                if member == "push" or member == "append": return lambda x: target.append(x) or target
                if member == "pop": return lambda idx=-1: target.pop(idx)
                if member == "join": return lambda sep=" ": sep.join(str(x) for x in target)
                if member == "contains": return lambda x: x in target
                if member == "reverse": return lambda: list(reversed(target))
                if member == "sort": return lambda reverse=False: sorted(target, reverse=bool(reverse))
                if member == "slice": return lambda start, end=None: target[int(start):int(end) if end is not None else None]
                if member == "count": return lambda x: target.count(x)
                if member == "indexOf" or member == "find_index": return lambda x: target.index(x) if x in target else -1
                if member == "map":
                    def _map(fn):
                        res = []
                        for x in target:
                            val = fn.call(self, [x]) if isinstance(fn, AiraFunction) else fn(x)
                            res.append(val)
                        return res
                    return _map
                if member == "filter":
                    def _filter(fn):
                        res = []
                        for x in target:
                            val = fn.call(self, [x]) if isinstance(fn, AiraFunction) else fn(x)
                            if val:
                                res.append(x)
                        return res
                    return _filter
                if member == "find":
                    def _find(fn):
                        for x in target:
                            val = fn.call(self, [x]) if isinstance(fn, AiraFunction) else fn(x)
                            if val:
                                return x
                        return None
                    return _find
                if member == "each" or member == "forEach":
                    def _each(fn):
                        for x in target:
                            if isinstance(fn, AiraFunction):
                                fn.call(self, [x])
                            else:
                                fn(x)
                        return target
                    return _each
                if member == "reduce":
                    def _reduce(fn, initial=None):
                        items = list(target)
                        if initial is None:
                            if not items:
                                raise ValueError("reduce on empty list with no initial value")
                            accum = items[0]
                            items = items[1:]
                        else:
                            accum = initial
                        for x in items:
                            if isinstance(fn, AiraFunction):
                                accum = fn.call(self, [accum, x])
                            else:
                                accum = fn(accum, x)
                        return accum
                    return _reduce

            elif hasattr(target, member):
                return getattr(target, member)

            raise AttributeError(f"AiraLang RuntimeError: Object of type '{type(target).__name__}' has no member '{member}' (Line {node.line})")

        elif nodetype == "IndexAccessNode":
            target = self.eval_node(node.target, env)
            idx = self.eval_node(node.index, env)
            try:
                return target[idx]
            except Exception as e:
                raise IndexError(f"AiraLang IndexError: Failed to access index '{idx}' on '{target}' (Line {node.line}): {e}")

        elif nodetype == "UnaryOpNode":
            val = self.eval_node(node.right, env)
            if node.op == "-":
                return -val
            elif node.op in ("!", "not"):
                return not val
            raise ValueError(f"Unknown unary op {node.op}")

        elif nodetype == "BinaryOpNode":
            left = self.eval_node(node.left, env)
            right = self.eval_node(node.right, env)

            if node.op == "+":
                if isinstance(left, str) or isinstance(right, str):
                    return str(left) + str(right)
                return left + right

            if node.op == "-": return left - right
            if node.op == "*": return left * right
            if node.op == "/": return left / right
            if node.op == "%": return left % right
            if node.op == "==": return left == right
            if node.op in ("!=", "≠"): return left != right
            if node.op == "<": return left < right
            if node.op == ">": return left > right
            if node.op == "<=": return left <= right
            if node.op == ">=": return left >= right
            if node.op == "and": return left and right
            if node.op == "or": return left or right
            raise ValueError(f"Unknown binary op {node.op}")

        elif nodetype == "FunctionCallNode":
            callee = self.eval_node(node.callee, env)
            args = []
            kwargs = {}
            for arg in node.arguments:
                if getattr(arg, "__class__", None).__name__ == "NamedArgNode":
                    kwargs[arg.name] = self.eval_node(arg.value, env)
                else:
                    args.append(self.eval_node(arg, env))

            if isinstance(callee, AiraFunction):
                return callee.call(self, args, kwargs=kwargs)
            elif isinstance(callee, AiraClass):
                return callee.instantiate(self, args, kwargs=kwargs)
            elif callable(callee):
                if kwargs:
                    return callee(*args, **kwargs)
                return callee(*args)
            else:
                raise TypeError(f"AiraLang RuntimeError: '{callee}' is not callable (Line {node.line})")

        raise NotImplementedError(f"AiraLang Evaluator: Unhandled AST Node '{nodetype}'")
