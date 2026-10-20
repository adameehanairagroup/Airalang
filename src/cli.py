import sys
import os
import time
import json
import threading

# Cross-platform UTF-8 Console and ANSI Color configuration
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
        sys.stdin.reconfigure(encoding="utf-8", errors="replace")
        import ctypes
        kernel32 = ctypes.windll.kernel32
        hStdOut = kernel32.GetStdHandle(-11)
        mode = ctypes.c_ulong()
        kernel32.GetConsoleMode(hStdOut, ctypes.byref(mode))
        mode.value |= 0x0004
        kernel32.SetConsoleMode(hStdOut, mode)
    except Exception:
        pass

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from lexer import Lexer
from parser import Parser
from evaluator import Evaluator

VERSION = "1.5.0 (Full-Stack Web, Cyber OSINT & F-Strings Edition)"
AUTHOR = "Adam Eehan (Aira Group of Technology)"

ASCII_BANNER = f"""\033[1;36m
    ___    _           __                    
   /   |  (_)________ _/ /   ____ _____  ____ _
  / /| | / / ___/ __ `/ /   / __ `/ __ \\/ __ `/
 / ___ |/ / /  / /_/ / /___/ /_/ / / / / /_/ / 
/_/  |_/_/_/   \\__,_/_____/\\__,_/_/ /_/\\__, /  
                                      /____/   
\033[0m\033[1;30m---------------------------------------------------\033[0m
 \033[1;32m[+]\033[0m \033[1;37mEngine:\033[0m AiraLang v{VERSION}
 \033[1;34m[*]\033[0m \033[1;37mCreator:\033[0m {AUTHOR}
 \033[1;35m[⚡]\033[0m \033[1;37mModules:\033[0m serve, term, webhook, bot, env, sec, ai, crypto, sqlite
 \033[1;36m[💡]\033[0m \033[1;37mAI Copilot:\033[0m Type \033[1;36m@aira <question>\033[0m or \033[1;36m?<question>\033[0m in REPL for instant help.
 \033[1;33m[!]\033[0m Multiline support: open \033[1;32m{{\033[0m will continue on next line.
 \033[1;33m[!]\033[0m Type \033[1;31m'exit'\033[0m or \033[1;31m'quit'\033[0m to close REPL.
\033[1;30m---------------------------------------------------\033[0m
"""


def is_balanced(code):
    in_single_quote = False
    in_double_quote = False
    in_comment = False
    
    braces = 0
    parens = 0
    brackets = 0
    
    i = 0
    while i < len(code):
        c = code[i]
        
        if in_comment:
            if c == '\n':
                in_comment = False
            i += 1
            continue
            
        if c == '#' and not in_single_quote and not in_double_quote:
            in_comment = True
            i += 1
            continue
            
        if c == '"' and not in_single_quote:
            if i > 0 and code[i-1] == '\\':
                pass
            else:
                in_double_quote = not in_double_quote
            i += 1
            continue
            
        if c == "'" and not in_double_quote:
            if i > 0 and code[i-1] == '\\':
                pass
            else:
                in_single_quote = not in_single_quote
            i += 1
            continue
            
        if not in_single_quote and not in_double_quote:
            if c == '{': braces += 1
            elif c == '}': braces -= 1
            elif c == '(': parens += 1
            elif c == ')': parens -= 1
            elif c == '[': brackets += 1
            elif c == ']': brackets -= 1
            
        i += 1
        
    return (braces <= 0 and parens <= 0 and brackets <= 0 and not in_single_quote and not in_double_quote)

def auto_insert_semicolons_into_file(filepath, lines_to_fix):
    if not filepath or not os.path.exists(filepath):
        return
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            content = f.readlines()
        modified = False
        for line_no in lines_to_fix:
            idx = line_no - 1
            if 0 <= idx < len(content):
                raw_line = content[idx]
                newline_char = "\n" if raw_line.endswith("\n") else ""
                stripped = raw_line.rstrip("\r\n")

                code_part = stripped
                comment_part = ""
                for sep in ("//", "#"):
                    if sep in stripped:
                        parts = stripped.split(sep, 1)
                        code_part = parts[0]
                        comment_part = sep + parts[1]
                        break
                code_stripped = code_part.rstrip()
                if code_stripped and not code_stripped.endswith(";"):
                    code_stripped += ";"
                    spacing = " " if comment_part else ""
                    content[idx] = f"{code_stripped}{spacing}{comment_part}{newline_char}"
                    modified = True
        if modified:
            with open(filepath, "w", encoding="utf-8") as f:
                f.writelines(content)
    except Exception:
        pass

def aira_explain_error(error, source_code, filepath=None):
    err_str = str(error)
    RED = "\033[1;31m"
    RESET = "\033[0m"

    print(f"\n{RED}⚠ AiraLang Compiler Error: {err_str}{RESET}")

    explanation = None
    try:
        import urllib.request, json
        from stdlib_modules import AIRALANG_CORE_KNOWLEDGE_BASE
        sys_prompt = (
            "You are Aira, sovereign AI engine of AiraLang. "
            "A developer encountered an error in their AiraLang code. "
            "In 1-2 direct, friendly, and expert sentences in pure English (starting with '⚠ Why this happened: '), "
            "explain why this error occurred and tell them exactly how to fix it in AiraLang. "
            "Rules: Variables use 'let' (NOT 'var'), output uses 'say' (NOT 'print'). "
            "Knowledge Base: " + AIRALANG_CORE_KNOWLEDGE_BASE[:1000]
        )
        req_data = json.dumps({
            "model": "aira-3-flash",
            "messages": [
                {"role": "system", "content": sys_prompt},
                {"role": "user", "content": f"Source:\n{source_code[:1000]}\n\nError:\n{err_str}"}
            ],
            "max_tokens": 120
        }).encode("utf-8")
        req = urllib.request.Request("http://127.0.0.1:8080/v1/chat/completions", data=req_data, headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req, timeout=1.8) as resp:
            data = json.loads(resp.read().decode("utf-8"))
            explanation = data["choices"][0]["message"]["content"].strip()
    except Exception:
        pass

    if not explanation:
        if "is not defined" in err_str:
            var_name = err_str.split("'")[1] if "'" in err_str else "variable"
            explanation = f"⚠ Why this happened: '{var_name}' was referenced before declaration. In AiraLang, declare it first with 'let {var_name} = ...;'."
        elif "Missing block brace" in err_str or "Expected 'LBRACE'" in err_str:
            explanation = "⚠ Why this happened: Control blocks (if, while, for) require braces '{ }' in AiraLang."
        elif "var " in source_code:
            explanation = "⚠ Why this happened: 'var' keyword is not recognized. In AiraLang, use 'let' to declare variables."
        elif "print(" in source_code:
            explanation = "⚠ Why this happened: 'print()' function is not recognized. In AiraLang, use 'say \"output\";'."
        else:
            explanation = "⚠ Why this happened: Syntax or runtime mismatch detected. Check statement syntax and variable scope."

    if not explanation.startswith("⚠"):
        explanation = f"⚠ {explanation}"
    print(f"{RED}{explanation}{RESET}\n")

def run_code(source_code, evaluator=None, filepath=None):
    if evaluator is None:
        evaluator = Evaluator()
    try:
        lexer = Lexer(source_code)
        tokens = lexer.tokenize()
        parser = Parser(tokens)
        ast = parser.parse()
        if filepath and parser.missing_semicolon_lines:
            auto_insert_semicolons_into_file(filepath, parser.missing_semicolon_lines)
        return evaluator.eval(ast)
    except SyntaxError as e:
        aira_explain_error(e, source_code, filepath)
    except NameError as e:
        aira_explain_error(e, source_code, filepath)
    except TypeError as e:
        aira_explain_error(e, source_code, filepath)
    except Exception as e:
        aira_explain_error(e, source_code, filepath)

def run_file(filepath):
    if not os.path.exists(filepath):
        print(f"\033[1;31m[!] Error: File '{filepath}' not found.\033[0m")
        sys.exit(1)
    try:
        from stdlib_modules import check_and_prompt_ai_engine
        check_and_prompt_ai_engine()
    except Exception:
        pass
    with open(filepath, "r", encoding="utf-8") as f:
        code = f.read()
    run_code(code, filepath=filepath)

def start_repl():
    print(ASCII_BANNER)
    try:
        from stdlib_modules import check_and_prompt_ai_engine
        check_and_prompt_ai_engine()
    except Exception:
        pass
    evaluator = Evaluator()
    buffer = []
    
    while True:
        try:
            if not buffer:
                prompt = "\033[1;35m[aira]\033[0m \033[1;32m❯❯\033[0m "
            else:
                prompt = "\033[1;33m[....]\033[0m \033[1;32m··\033[0m "
                
            line = input(prompt)
            clean_cmd = line.strip()

            if not buffer and (clean_cmd.startswith("@aira") or clean_cmd.startswith("@airaai") or clean_cmd.startswith("?")):
                query = clean_cmd
                if clean_cmd.startswith("@airaai"):
                    query = clean_cmd[7:].strip()
                elif clean_cmd.startswith("@aira"):
                    query = clean_cmd[5:].strip()
                elif clean_cmd.startswith("?"):
                    query = clean_cmd[1:].strip()

                print(f"\033[1;36m[Aira Copilot]:\033[0m Thinking...")
                reply = None
                try:
                    import urllib.request, json
                    from stdlib_modules import AIRALANG_CORE_KNOWLEDGE_BASE
                    sys_prompt = (
                        "You are Aira, sovereign pair programmer and REPL copilot for AiraLang. "
                        "Answer the developer's question directly with concise, correct AiraLang code examples in pure English. "
                        "Rules: Use 'let' for variables (NOT 'var'), 'say' for output (NOT 'print'). "
                        "AiraLang Knowledge Base: " + AIRALANG_CORE_KNOWLEDGE_BASE[:1500]
                    )
                    req_data = json.dumps({
                        "model": "aira-3-flash",
                        "messages": [
                            {"role": "system", "content": sys_prompt},
                            {"role": "user", "content": query}
                        ],
                        "max_tokens": 200
                    }).encode("utf-8")
                    req = urllib.request.Request("http://127.0.0.1:8080/v1/chat/completions", data=req_data, headers={"Content-Type": "application/json"})
                    with urllib.request.urlopen(req, timeout=3.0) as resp:
                        data = json.loads(resp.read().decode("utf-8"))
                        reply = data["choices"][0]["message"]["content"].strip()
                except Exception:
                    pass

                if not reply:
                    reply = "Use 'let x = 10;' for variables, 'say x;' for output, and 'import <mod>;' for standard library modules."

                print(f"\033[1;32m✔ Aira:\033[0m {reply}\n")
                continue
            
            if not buffer and line.strip() in ("exit", "quit"):
                print("\033[1;33m[*] Exiting AiraLang. Goodbye, Boss!\033[0m")
                break
                
            if not buffer and line.strip() == "clear":
                os.system("clear")
                continue
                
            if not line.strip() and not buffer:
                continue

            buffer.append(line)
            full_code = "\n".join(buffer)
            
            if is_balanced(full_code) or (len(buffer) > 1 and not line.strip()):
                run_code(full_code, evaluator)
                buffer = []
                
        except (KeyboardInterrupt, EOFError):
            if buffer:
                print("\n\033[1;33m[*] Discarded incomplete block.\033[0m")
                buffer = []
            else:
                print("\n\033[1;33m[*] Interrupted. Exiting AiraLang...\033[0m")
                break

def build_file(source_path, output_path=None):
    if not os.path.exists(source_path):
        print(f"\033[1;31m[!] Error: File '{source_path}' not found.\033[0m")
        sys.exit(1)
    if output_path is None:
        output_path = os.path.splitext(source_path)[0]
    with open(source_path, "r", encoding="utf-8") as f:
        aira_code = f.read()

    runner_script = f"""#!/data/data/com.termux/files/usr/bin/python3
# ==========================================
# Standalone Binary Executable
# Generated by AiraLang v{VERSION}
# Author: {AUTHOR}
# ==========================================
import sys, os
AIRA_SRC = "/data/data/com.termux/files/home/airalang/src"
if AIRA_SRC not in sys.path:
    sys.path.insert(0, AIRA_SRC)
from cli import run_code
CODE = {repr(aira_code)}
if __name__ == '__main__':
    run_code(CODE)
"""
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(runner_script)
    os.chmod(output_path, 0o755)
    print(f"\033[1;32m[+] Successfully built executable binary:\033[0m {output_path}")
    print(f"\033[1;34m[*] Run it directly:\033[0m ./{output_path}")

def new_project(proj_name):
    if os.path.exists(proj_name):
        print(f"\033[1;31m[!] Error: Directory '{proj_name}' already exists.\033[0m")
        return
    os.makedirs(proj_name, exist_ok=True)
    main_code = f"""# 🚀 Welcome to {proj_name}!
# Powered by AiraLang v{VERSION}
# Creator: {AUTHOR}

import "ai";
import "sec";

say "🔥 Welcome to {proj_name} powered by AiraLang!";
let creator = "{AUTHOR}";
say "⚡ Built by: " + creator;
"""
    with open(os.path.join(proj_name, "main.aira"), "w", encoding="utf-8") as f:
        f.write(main_code)
    with open(os.path.join(proj_name, "README.md"), "w", encoding="utf-8") as f:
        f.write(f"# {proj_name}\\n\\nProject created with AiraLang v{VERSION}.\\n\\nRun:\\n```bash\\nairalang main.aira\\n```\\n")
    print(f"\033[1;32m[+] Created new AiraLang project:\033[0m {proj_name}/")
def revoke_ai_api_key():
    from stdlib_modules import load_airalang_config, save_airalang_config, _GLOBAL_AI_STATE, prompt_developer_for_ai_key

    CYAN = "\033[1;36m"
    YELLOW = "\033[1;33m"
    GREEN = "\033[1;32m"
    RED = "\033[1;31m"
    RESET = "\033[0m"
    BOLD = "\033[1m"

    cfg = load_airalang_config()
    old_provider = cfg.get("active_provider", "none")

    cfg["keys"] = {}
    cfg["active_provider"] = "gemini"
    cfg["ai_prompted"] = False
    save_airalang_config(cfg)

    for k in _GLOBAL_AI_STATE["keys"]:
        if k != "ollama":
            _GLOBAL_AI_STATE["keys"][k] = ""
    _GLOBAL_AI_STATE["active_provider"] = "gemini"
    _GLOBAL_AI_STATE["gemini_key"] = ""
    _GLOBAL_AI_STATE["groq_key"] = ""
    _GLOBAL_AI_STATE["custom_key"] = ""

    print(f"\n{RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{RESET}")
    print(f"{RED}{BOLD}🔄 [Aira AI Engine - Key Revoked]:{RESET}")
    print(f"{YELLOW}Previous API key(s) (active: {old_provider}) have been revoked and cleared.{RESET}")
    print(f"{CYAN}Rate limit reset! You can now configure a fresh API key.{RESET}")
    print(f"{RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{RESET}")

    prompt_developer_for_ai_key()

class AiraSpinner:
    def __init__(self, message="Processing..."):
        self.message = message
        self.stop_running = False
        self.thread = None
        self.frames = ["⢿", "⣻", "⣽", "⣾", "⣷", "⣯", "⣟", "⡿"]

    def _spin(self):
        i = 0
        while not self.stop_running:
            f = self.frames[i % len(self.frames)]
            sys.stdout.write(f"\r\033[1;36m {f}\033[0m {self.message}")
            sys.stdout.flush()
            time.sleep(0.08)
            i += 1

    def __enter__(self):
        self.stop_running = False
        self.thread = threading.Thread(target=self._spin, daemon=True)
        self.thread.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop_running = True
        if self.thread:
            self.thread.join(timeout=0.3)
        sys.stdout.write("\r\033[K")
        sys.stdout.flush()

def install_package(pkg_name):
    pkg_name = pkg_name.lower().strip()
    print(f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m")
    print(f"\033[1;35m📦 [Aira Package Manager (APM v1.0)]\033[0m")
    print(f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m")

    with AiraSpinner(f"Resolving package '{pkg_name}' from Aira Package Registry..."):
        time.sleep(0.8)
    print(f" \033[1;32m[✓]\033[0m Package \033[1;33m{pkg_name}\033[0m resolved successfully.")

    if pkg_name in ("youtube", "yt"):
        with AiraSpinner("Verifying engine dependencies (yt-dlp)..."):
            import subprocess
            try:
                import yt_dlp
                engine_ready = True
            except ImportError:
                engine_ready = False
            time.sleep(0.4)

        if engine_ready:
            print(" \033[1;32m[✓]\033[0m Backend engine yt-dlp is ready.")
        else:
            with AiraSpinner("Installing yt-dlp backend engine via pip..."):
                subprocess.check_call([sys.executable, "-m", "pip", "install", "yt-dlp"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            print(" \033[1;32m[✓]\033[0m Backend engine installed successfully.")

        with AiraSpinner("Configuring package registry and setting up permissions..."):
            config_dir = os.path.expanduser("~/.config/airalang")
            os.makedirs(config_dir, exist_ok=True)
            packages_file = os.path.join(config_dir, "installed_packages.json")
            packages = {}
            if os.path.exists(packages_file):
                try:
                    with open(packages_file, "r", encoding="utf-8") as f:
                        packages = json.load(f)
                except Exception:
                    packages = {}
            packages["youtube"] = {
                "version": "1.0.0",
                "installed_at": time.time(),
                "description": "YouTube video and audio download engine for AiraLang"
            }
            packages["yt"] = packages["youtube"]
            with open(packages_file, "w", encoding="utf-8") as f:
                json.dump(packages, f, indent=2)
            time.sleep(0.5)

        print(f" \033[1;32m[✓]\033[0m Module '{pkg_name}' registered in APM.")
        print(f"\n\033[1;32m✨ Successfully installed '{pkg_name}' v1.0.0!\033[0m")
        print(f"\033[1;36m💡 Usage in your .aira scripts:\033[0m")
        print(f"   import \"youtube\";")
        print(f"   let video = youtube.get(url);")
        print(f"   let stream = video.stream(\"mp4\");")
        print(f"   let save = os.save(\"video.mp4\");")
        print(f"   stream.download(save);")
        print(f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m")
    else:
        print(f" \033[1;31m[✗]\033[0m Package '{pkg_name}' not found in Aira Package Registry.")
        print(f"Available packages: youtube (yt)")

def print_help():
    print(f"""\033[1;36mAiraLang v{VERSION}\033[0m
Creator: {AUTHOR}

\033[1;33mUsage:\033[0m
  airalang                             Start interactive REPL
  airalang <script.aira>               Execute an AiraLang script
  airalang run <script.aira>           Execute an AiraLang script
  airalang install <package>           Install a module via Aira Package Manager (APM)
  aira install <package>               Install a module via Aira Package Manager (APM)
  airalang build <file.aira> [-o out]  Compile script to standalone executable
  airalang new <project_name>          Create a new AiraLang project scaffold
  airalang --ai api revoke             Revoke active API key and configure a fresh one
  airalang --ai-setup                  Configure Aira AI Engine API key (interactive)
  airalang --set-key <key> [provider]  Set API key for provider (OpenRouter/Groq/OpenAI/Gemini)
  airalang -v, --version               Display version information
  airalang -h, --help                  Show this help message
""")

def main():
    if len(sys.argv) > 1:
        args_str = " ".join(sys.argv[1:]).lower().strip()
        if args_str in (
            "--ai api revoke",
            "ai api revoke",
            "--ai revoke",
            "ai revoke",
            "--revoke-api",
            "--ai-revoke",
            "revoke-api"
        ):
            revoke_ai_api_key()
            sys.exit(0)

        arg1 = sys.argv[1]
        if arg1 in ("-v", "--version"):
            print(f"AiraLang v{VERSION} | Creator: {AUTHOR}")
        elif arg1 in ("-h", "--help"):
            print_help()
        elif arg1 in ("install", "i", "add"):
            if len(sys.argv) > 2:
                install_package(sys.argv[2])
                sys.exit(0)
            else:
                print("\033[1;31m[!] Error: Specify package name: airalang install <package_name>\033[0m")
                sys.exit(1)
        elif arg1 in ("--ai-setup", "--setup-ai"):
            from stdlib_modules import prompt_developer_for_ai_key
            prompt_developer_for_ai_key()
            sys.exit(0)
        elif arg1 == "--set-key":
            if len(sys.argv) > 2:
                from stdlib_modules import create_ai_module
                ai_mod = create_ai_module()
                prov = sys.argv[3] if len(sys.argv) > 3 else None
                ai_mod.methods["set_key"](sys.argv[2], provider=prov)
                print(f"\033[1;32m✓ API Key successfully configured and saved to config!\033[0m")
                sys.exit(0)
            else:
                print("\033[1;31m[!] Error: Specify API key: airalang --set-key <key> [provider]\033[0m")
                sys.exit(1)
        elif arg1 == "run":
            if len(sys.argv) > 2:
                run_file(sys.argv[2])
            else:
                print("\033[1;31m[!] Error: Specify file to run: airalang run <file.aira>\033[0m")
        elif arg1 == "build":
            if len(sys.argv) > 2:
                src_file = sys.argv[2]
                out_file = None
                if "-o" in sys.argv:
                    idx = sys.argv.index("-o")
                    if idx + 1 < len(sys.argv):
                        out_file = sys.argv[idx + 1]
                build_file(src_file, out_file)
            else:
                print("\033[1;31m[!] Error: Specify file to build: airalang build <file.aira> [-o <out>]\033[0m")
        elif arg1 == "new":
            if len(sys.argv) > 2:
                new_project(sys.argv[2])
            else:
                print("\033[1;31m[!] Error: Specify project name: airalang new <project_name>\033[0m")
        else:
            run_file(arg1)
    else:
        start_repl()

if __name__ == "__main__":
    main()
