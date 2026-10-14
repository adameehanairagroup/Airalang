import os
import sys
import math
import time
import json
import random
import hashlib
import base64
import socket
import sqlite3
import urllib.request
import urllib.parse
import urllib.error
import threading
import struct
import zipfile
import re
import subprocess
import ssl
import hmac
import uuid
import datetime
from concurrent.futures import ThreadPoolExecutor

class AiraModule:
    def __init__(self, name, methods=None):
        self.name = name
        self.methods = methods or {}

    def get(self, prop_name):
        if prop_name in self.methods:
            return self.methods[prop_name]
        raise AttributeError(f"AiraLang Module '{self.name}' has no method or property '{prop_name}'")

class AiraSQLiteDB:
    def __init__(self, db_path):
        self.db_path = db_path
        self.conn = sqlite3.connect(db_path)
        self.conn.row_factory = sqlite3.Row

    def execute(self, sql, params=None):
        cursor = self.conn.cursor()
        if params:
            cursor.execute(sql, tuple(params) if isinstance(params, list) else (params,))
        else:
            cursor.execute(sql)
        self.conn.commit()
        return cursor.rowcount

    def query(self, sql, params=None):
        cursor = self.conn.cursor()
        if params:
            cursor.execute(sql, tuple(params) if isinstance(params, list) else (params,))
        else:
            cursor.execute(sql)
        rows = cursor.fetchall()
        result = []
        for r in rows:
            result.append({k: r[k] for k in r.keys()})
        return result

    def insert(self, table, data_dict):
        if not isinstance(data_dict, dict):
            raise TypeError("sqlite.insert requires a dictionary of column: value")
        columns = ", ".join(data_dict.keys())
        placeholders = ", ".join(["?"] * len(data_dict))
        sql = f"INSERT INTO {table} ({columns}) VALUES ({placeholders})"
        return self.execute(sql, list(data_dict.values()))

    def close(self):
        self.conn.close()
        return True

    def get(self, prop_name):
        if hasattr(self, prop_name):
            return getattr(self, prop_name)
        raise AttributeError(f"SQLite DB object has no property '{prop_name}'")

def create_sqlite_module():
    def open_db(path):
        db = AiraSQLiteDB(path)
        return AiraModule("sqlite_conn", {
            "execute": db.execute,
            "query": db.query,
            "insert": db.insert,
            "close": db.close
        })

    return AiraModule("sqlite", {
        "open": open_db,
        "connect": open_db
    })

def create_net_module():
    def scan_port(host, port, timeout=2.0):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(float(timeout))
            result = s.connect_ex((str(host), int(port)))
            s.close()
            return result == 0
        except Exception:
            return False

    def scan_ports(host, start_port, end_port, timeout=1.0):
        open_ports = []
        for p in range(int(start_port), int(end_port) + 1):
            if scan_port(host, p, timeout):
                open_ports.append(p)
        return open_ports

    def resolve(domain):
        try:
            return socket.gethostbyname(str(domain))
        except Exception as e:
            return f"Error: {e}"

    def local_ip():
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
            s.close()
            return ip
        except Exception:
            return "127.0.0.1"

    def tcp_send(host, port, message, timeout=5.0):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(float(timeout))
            s.connect((str(host), int(port)))
            s.sendall(str(message).encode('utf-8'))
            data = s.recv(4096)
            s.close()
            return data.decode('utf-8', errors='ignore')
        except Exception as e:
            return f"Socket Error: {e}"

    return AiraModule("net", {
        "scan_port": scan_port,
        "scan_ports": scan_ports,
        "resolve": resolve,
        "local_ip": local_ip,
        "tcp_send": tcp_send
    })

def create_file_module():
    def read_file(path):
        with open(path, "r", encoding="utf-8") as f:
            return f.read()

    def write_file(path, content):
        with open(path, "w", encoding="utf-8") as f:
            f.write(str(content))
        return True

    def append_file(path, content):
        with open(path, "a", encoding="utf-8") as f:
            f.write(str(content))
        return True

    def exists(path):
        return os.path.exists(path)

    def delete(path):
        if os.path.exists(path):
            os.remove(path)
            return True
        return False

    def size(path):
        if os.path.exists(path):
            return os.path.getsize(path)
        return -1

    def save_path(filepath):
        path_str = str(filepath).strip()
        dir_name = os.path.dirname(path_str)
        if dir_name:
            os.makedirs(dir_name, exist_ok=True)
        return path_str

    return AiraModule("file", {
        "read": read_file,
        "write": write_file,
        "append": append_file,
        "exists": exists,
        "delete": delete,
        "size": size,
        "save": save_path
    })

def create_os_module():
    def cmd(command):
        return os.popen(command).read().strip()

    def env(var_name, default=""):
        return os.environ.get(var_name, default)

    def platform():
        return sys.platform

    def cwd():
        return os.getcwd()

    def listdir(path="."):
        return os.listdir(path)

    def mkdir(path):
        os.makedirs(path, exist_ok=True)
        return True

    def save_path(filepath):
        path_str = str(filepath).strip()
        dir_name = os.path.dirname(path_str)
        if dir_name:
            os.makedirs(dir_name, exist_ok=True)
        return path_str

    def get_args():
        if len(sys.argv) > 1:
            raw = sys.argv[1:]
            while raw and raw[0] in ("run", "build", "new", "-v", "--version", "-h", "--help"):
                raw = raw[1:]
            if raw and (raw[0].endswith(".aira") or raw[0].endswith(".py") or "airalang" in raw[0] or "cli.py" in raw[0]):
                raw = raw[1:]
            return raw
        return []

    def exit_app(code=0):
        sys.exit(int(code))

    return AiraModule("os", {
        "cmd": cmd,
        "system": cmd,
        "env": env,
        "getenv": env,
        "platform": platform,
        "cwd": cwd,
        "getcwd": cwd,
        "listdir": listdir,
        "mkdir": mkdir,
        "save": save_path,
        "args": get_args,
        "argv": get_args,
        "exit": exit_app,
        "is_windows": lambda: sys.platform == "win32",
        "is_android": lambda: "android" in sys.platform.lower() or os.path.exists("/data/data/com.termux"),
        "is_linux": lambda: sys.platform.startswith("linux") and not os.path.exists("/data/data/com.termux")
    })

def create_math_module():
    return AiraModule("math", {
        "sqrt": lambda x: math.sqrt(x),
        "floor": lambda x: math.floor(x),
        "ceil": lambda x: math.ceil(x),
        "abs": lambda x: abs(x),
        "round": lambda x, d=0: round(x, d) if d else round(x),
        "pow": lambda b, e: math.pow(b, e),
        "random": lambda low=0, high=1: random.randint(int(low), int(high)) if isinstance(low, int) and isinstance(high, int) else random.uniform(low, high),
        "pi": math.pi,
        "e": math.e
    })

def create_time_module():
    return AiraModule("time", {
        "now": lambda: time.strftime("%Y-%m-%d %H:%M:%S"),
        "time": lambda: time.time(),
        "sleep": lambda s: time.sleep(s)
    })

def create_json_module():
    return AiraModule("json", {
        "parse": lambda s: json.loads(s),
        "mkstring": lambda obj, indent=2: json.dumps(obj, indent=indent),
        "stringify": lambda obj, indent=2: json.dumps(obj, indent=indent)
    })

def create_http_module():
    def get(url, headers=None):
        req_headers = {"User-Agent": "AiraLang/1.3 (Termux; Android)"}
        if isinstance(headers, dict):
            req_headers.update(headers)
        req = urllib.request.Request(url, headers=req_headers)
        try:
            with urllib.request.urlopen(req, timeout=15) as response:
                return response.read().decode("utf-8")
        except Exception as e:
            raise RuntimeError(f"HTTP GET Error: {e}")

    def post(url, data_payload=None, headers=None):
        req_headers = {"User-Agent": "AiraLang/1.3 (Termux; Android)"}
        if isinstance(headers, dict):
            req_headers.update(headers)
        encoded_data = None
        if isinstance(data_payload, dict):
            req_headers["Content-Type"] = "application/x-www-form-urlencoded"
            encoded_data = urllib.parse.urlencode(data_payload).encode("utf-8")
        elif isinstance(data_payload, str):
            encoded_data = data_payload.encode("utf-8")
        req = urllib.request.Request(url, data=encoded_data, headers=req_headers)
        try:
            with urllib.request.urlopen(req, timeout=15) as response:
                return response.read().decode("utf-8")
        except Exception as e:
            raise RuntimeError(f"HTTP POST Error: {e}")

    def download(url, dest_path):
        req = urllib.request.Request(url, headers={"User-Agent": "AiraLang/1.3"})
        try:
            with urllib.request.urlopen(req, timeout=30) as response, open(dest_path, "wb") as out_file:
                out_file.write(response.read())
            return True
        except Exception as e:
            raise RuntimeError(f"HTTP Download Error: {e}")

    def status(url):
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "AiraLang/1.3"})
            with urllib.request.urlopen(req, timeout=10) as response:
                return response.getcode()
        except urllib.error.HTTPError as e:
            return e.code
        except Exception:
            return 0

    return AiraModule("http", {
        "get": get,
        "post": post,
        "download": download,
        "status": status
    })

def create_crypto_module():
    def md5(s):
        return hashlib.md5(str(s).encode('utf-8')).hexdigest()

    def sha256(s):
        return hashlib.sha256(str(s).encode('utf-8')).hexdigest()

    def sha1(s):
        return hashlib.sha1(str(s).encode('utf-8')).hexdigest()

    def b64encode(s):
        return base64.b64encode(str(s).encode('utf-8')).decode('utf-8')

    def b64decode(s):
        return base64.b64decode(str(s).encode('utf-8')).decode('utf-8')

    def hmac_sha256(key, message):
        return hmac.new(str(key).encode('utf-8'), str(message).encode('utf-8'), hashlib.sha256).hexdigest()

    def uuid_str():
        return str(uuid.uuid4())

    def jwt_sign(payload, secret, alg="HS256"):
        header = {"alg": alg, "typ": "JWT"}
        h_json = json.dumps(header, separators=(',', ':')).encode('utf-8')
        p_json = json.dumps(payload, separators=(',', ':')).encode('utf-8')
        h_b64 = base64.urlsafe_b64encode(h_json).decode('utf-8').rstrip('=')
        p_b64 = base64.urlsafe_b64encode(p_json).decode('utf-8').rstrip('=')
        signing_input = f"{h_b64}.{p_b64}".encode('utf-8')
        sig = hmac.new(str(secret).encode('utf-8'), signing_input, hashlib.sha256).digest()
        sig_b64 = base64.urlsafe_b64encode(sig).decode('utf-8').rstrip('=')
        return f"{h_b64}.{p_b64}.{sig_b64}"

    def jwt_verify(token, secret):
        parts = str(token).split('.')
        if len(parts) != 3:
            return {"valid": False, "error": "Invalid JWT format"}
        h_b64, p_b64, sig_b64 = parts
        signing_input = f"{h_b64}.{p_b64}".encode('utf-8')
        expected_sig = hmac.new(str(secret).encode('utf-8'), signing_input, hashlib.sha256).digest()
        pad = '=' * (-len(sig_b64) % 4)
        actual_sig = base64.urlsafe_b64decode(sig_b64 + pad)
        if not hmac.compare_digest(expected_sig, actual_sig):
            return {"valid": False, "error": "Invalid signature"}
        pad_p = '=' * (-len(p_b64) % 4)
        payload_data = json.loads(base64.urlsafe_b64decode(p_b64 + pad_p).decode('utf-8'))
        return {"valid": True, "payload": payload_data}

    return AiraModule("crypto", {
        "md5": md5,
        "sha256": sha256,
        "sha1": sha1,
        "base64_encode": b64encode,
        "base64_decode": b64decode,
        "b64encode": b64encode,
        "b64decode": b64decode,
        "hmac": hmac_sha256,
        "uuid": uuid_str,
        "jwt_sign": jwt_sign,
        "jwt_verify": jwt_verify
    })


def create_string_module():
    return AiraModule("string", {
        "upper": lambda s: str(s).upper(),
        "lower": lambda s: str(s).lower(),
        "split": lambda s, sep=" ": str(s).split(sep),
        "replace": lambda s, old, new: str(s).replace(str(old), str(new)),
        "trim": lambda s: str(s).strip(),
        "contains": lambda s, sub: str(sub) in str(s),
        "length": lambda s: len(str(s)),
        "join": lambda lst, sep=" ": sep.join(str(x) for x in lst)
    })

AIRA_DEFAULT_SYSTEM_PROMPT = (
    "You are Aira, the official AI engine of AiraLang, engineered and created by Adam Eehan "
    "(Founder & CEO, Aira Group of Technology). You are brilliant, helpful, witty, "
    "and fiercely loyal. Regardless of which backend API or model is processing this request, "
    "never identify as ChatGPT, OpenAI, Claude, LLaMA, or Gemini. Always identify proudly as Aira AI."
)

WORLD_AI_PROVIDERS = {
    "argon": {
        "name": "Google Gemini 4 Argon (Reasoning Flagship)",
        "type": "gemini",
        "default_model": "gemini-4-argon",
        "env_var": "GEMINI_API_KEY"
    },
    "gemini": {
        "name": "Google Gemini",
        "type": "gemini",
        "default_model": "gemini-1.5-flash",
        "env_var": "GEMINI_API_KEY"
    },
    "groq": {
        "name": "Groq Cloud (Ultra-Fast)",
        "type": "openai_compat",
        "url": "https://api.groq.com/openai/v1/chat/completions",
        "default_model": "openai/gpt-oss-120b",
        "env_var": "GROQ_API_KEY"
    },
    "openai": {
        "name": "OpenAI",
        "type": "openai_compat",
        "url": "https://api.openai.com/v1/chat/completions",
        "default_model": "gpt-4o-mini",
        "env_var": "OPENAI_API_KEY"
    },
    "deepseek": {
        "name": "DeepSeek AI",
        "type": "openai_compat",
        "url": "https://api.deepseek.com/chat/completions",
        "default_model": "deepseek-chat",
        "env_var": "DEEPSEEK_API_KEY"
    },
    "openrouter": {
        "name": "OpenRouter (300+ Universal Models)",
        "type": "openai_compat",
        "url": "https://openrouter.ai/api/v1/chat/completions",
        "default_model": "meta-llama/llama-3.3-70b-instruct",
        "env_var": "OPENROUTER_API_KEY"
    },
    "anthropic": {
        "name": "Anthropic Claude",
        "type": "anthropic",
        "url": "https://api.anthropic.com/v1/messages",
        "default_model": "claude-3-5-sonnet-20241022",
        "env_var": "ANTHROPIC_API_KEY"
    },
    "mistral": {
        "name": "Mistral AI",
        "type": "openai_compat",
        "url": "https://api.mistral.ai/v1/chat/completions",
        "default_model": "mistral-small-latest",
        "env_var": "MISTRAL_API_KEY"
    },
    "ollama": {
        "name": "Ollama (Local / Offline)",
        "type": "openai_compat",
        "url": "http://localhost:11434/v1/chat/completions",
        "default_model": "llama3.2",
        "env_var": "OLLAMA_HOST"
    },
    "together": {
        "name": "Together AI",
        "type": "openai_compat",
        "url": "https://api.together.xyz/v1/chat/completions",
        "default_model": "meta-llama/Llama-3.3-70B-Instruct-Turbo",
        "env_var": "TOGETHER_API_KEY"
    },
    "perplexity": {
        "name": "Perplexity AI",
        "type": "openai_compat",
        "url": "https://api.perplexity.ai/chat/completions",
        "default_model": "sonar",
        "env_var": "PERPLEXITY_API_KEY"
    },
    "cerebras": {
        "name": "Cerebras Fast Inference",
        "type": "openai_compat",
        "url": "https://api.cerebras.ai/v1/chat/completions",
        "default_model": "llama3.3-70b",
        "env_var": "CEREBRAS_API_KEY"
    }
}

AIRALANG_CONFIG_DIR = os.path.expanduser("~/.config/airalang")
AIRALANG_CONFIG_FILE = os.path.join(AIRALANG_CONFIG_DIR, "config.json")

def load_airalang_config():
    if os.path.exists(AIRALANG_CONFIG_FILE):
        try:
            with open(AIRALANG_CONFIG_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception:
            return {}
    return {}

def save_airalang_config(config_data):
    try:
        os.makedirs(AIRALANG_CONFIG_DIR, exist_ok=True)
        with open(AIRALANG_CONFIG_FILE, "w", encoding="utf-8") as f:
            json.dump(config_data, f, indent=2)
        return True
    except Exception:
        return False

_GLOBAL_AI_STATE = {
    "active_provider": "gemini",
    "active_model": "",
    "custom_endpoint": "",
    "custom_key": "",
    "system_prompt": AIRA_DEFAULT_SYSTEM_PROMPT,
    "keys": {
        "gemini": os.environ.get("GEMINI_API_KEY", ""),
        "groq": os.environ.get("GROQ_API_KEY", ""),
        "openai": os.environ.get("OPENAI_API_KEY", ""),
        "deepseek": os.environ.get("DEEPSEEK_API_KEY", ""),
        "openrouter": os.environ.get("OPENROUTER_API_KEY", ""),
        "anthropic": os.environ.get("ANTHROPIC_API_KEY", ""),
        "mistral": os.environ.get("MISTRAL_API_KEY", ""),
        "together": os.environ.get("TOGETHER_API_KEY", ""),
        "perplexity": os.environ.get("PERPLEXITY_API_KEY", ""),
        "cerebras": os.environ.get("CEREBRAS_API_KEY", ""),
        "ollama": "local"
    }
}

# Auto-hydrate keys from ~/.config/airalang/config.json
_init_cfg = load_airalang_config()
_init_keys = _init_cfg.get("keys", {})
for _prov, _val in _init_keys.items():
    if _val and _prov in _GLOBAL_AI_STATE["keys"] and not _GLOBAL_AI_STATE["keys"][_prov]:
        _GLOBAL_AI_STATE["keys"][_prov] = _val
if _init_cfg.get("active_provider"):
    _GLOBAL_AI_STATE["active_provider"] = _init_cfg.get("active_provider")
if _init_cfg.get("active_model"):
    _GLOBAL_AI_STATE["active_model"] = _init_cfg.get("active_model")

_GLOBAL_AI_STATE["gemini_key"] = _GLOBAL_AI_STATE["keys"]["gemini"]
_GLOBAL_AI_STATE["groq_key"] = _GLOBAL_AI_STATE["keys"]["groq"]

def prompt_developer_for_ai_key():
    if not sys.stdin.isatty():
        return False

    CYAN = "\033[1;36m"
    YELLOW = "\033[1;33m"
    GREEN = "\033[1;32m"
    RESET = "\033[0m"
    BOLD = "\033[1m"

    print(f"\n{CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{RESET}")
    print(f"{CYAN}{BOLD}✨ [Aira AI Engine Setup - Aira Group Of Technology]:{RESET}")
    print(f"{YELLOW}Hey there! If you want to activate Aira AI Engine,{RESET}")
    print(f"please paste your API key from any provider:")
    print(f"{BOLD}(OpenRouter / Groq / Gemini / OpenAI / DeepSeek / Anthropic){RESET}")
    print(f"{CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{RESET}")
    try:
        user_key = input(f"{GREEN}👉 Paste API Key (press Enter to skip): {RESET}").strip()
    except (EOFError, KeyboardInterrupt):
        print()
        return False

    if user_key:
        detected_provider = "gemini"
        if user_key.startswith("gsk_"): detected_provider = "groq"
        elif user_key.startswith("sk-or-"): detected_provider = "openrouter"
        elif user_key.startswith("sk-ant-"): detected_provider = "anthropic"
        elif user_key.startswith("pplx-"): detected_provider = "perplexity"
        elif user_key.startswith("csk-"): detected_provider = "cerebras"
        elif user_key.startswith("sk-"): detected_provider = "openai"

        _GLOBAL_AI_STATE["keys"][detected_provider] = user_key
        _GLOBAL_AI_STATE["active_provider"] = detected_provider
        if detected_provider == "gemini": _GLOBAL_AI_STATE["gemini_key"] = user_key
        if detected_provider == "groq": _GLOBAL_AI_STATE["groq_key"] = user_key

        cfg = load_airalang_config()
        if "keys" not in cfg:
            cfg["keys"] = {}
        cfg["keys"][detected_provider] = user_key
        cfg["active_provider"] = detected_provider
        cfg["ai_prompted"] = True
        save_airalang_config(cfg)

        print(f"{GREEN}✓ Aira AI Engine activated! Provider auto-detected as '{detected_provider}'.{RESET}")
        print(f"{CYAN}Identity: Aira AI by Adam Eehan. Saved to ~/.config/airalang/config.json{RESET}\n")
        return True
    else:
        cfg = load_airalang_config()
        cfg["ai_prompted"] = True
        save_airalang_config(cfg)
        print(f"{YELLOW}ℹ️  Skipped. Aira AI running in offline mode. (Run 'airalang --ai-setup' anytime to activate){RESET}\n")
        return False

def check_and_prompt_ai_engine(force=False):
    cfg = load_airalang_config()
    has_key = any(v for k, v in _GLOBAL_AI_STATE["keys"].items() if k != "ollama" and v)
    if has_key and not force:
        return True
    if cfg.get("ai_prompted") and not force:
        return False
    return prompt_developer_for_ai_key()

def create_ai_module():
    state = _GLOBAL_AI_STATE

    def set_key(key, provider=None, model=None):
        clean = str(key).strip()
        p = str(provider).lower().strip() if provider else None

        if p and (p in WORLD_AI_PROVIDERS or p == "custom"):
            state["active_provider"] = p
            state["keys"][p] = clean
            if p == "gemini": state["gemini_key"] = clean
            if p == "groq": state["groq_key"] = clean
            if p == "custom": state["custom_key"] = clean
        elif clean.startswith("gsk_"):
            state["active_provider"] = "groq"
            state["keys"]["groq"] = clean
            state["groq_key"] = clean
        elif clean.startswith("sk-or-"):
            state["active_provider"] = "openrouter"
            state["keys"]["openrouter"] = clean
        elif clean.startswith("sk-ant-"):
            state["active_provider"] = "anthropic"
            state["keys"]["anthropic"] = clean
        elif clean.startswith("pplx-"):
            state["active_provider"] = "perplexity"
            state["keys"]["perplexity"] = clean
        elif clean.startswith("csk-"):
            state["active_provider"] = "cerebras"
            state["keys"]["cerebras"] = clean
        elif clean.startswith("sk-"):
            target_p = p if p in ("deepseek", "openai") else "openai"
            state["active_provider"] = target_p
            state["keys"][target_p] = clean
        else:
            state["active_provider"] = "gemini"
            state["keys"]["gemini"] = clean
            state["gemini_key"] = clean

        if model:
            state["active_model"] = str(model).strip()

        cfg = load_airalang_config()
        if "keys" not in cfg:
            cfg["keys"] = {}
        cfg["keys"][state["active_provider"]] = clean
        cfg["active_provider"] = state["active_provider"]
        if model:
            cfg["active_model"] = str(model).strip()
        save_airalang_config(cfg)
        return True

    def get_key(provider=None):
        p = str(provider).lower().strip() if provider else state.get("active_provider", "gemini")
        return state["keys"].get(p, state.get("custom_key", ""))

    def set_provider(provider, key=None, model=None):
        p = str(provider).lower().strip()
        if p in WORLD_AI_PROVIDERS or p == "custom":
            state["active_provider"] = p
            if key:
                clean_k = str(key).strip()
                state["keys"][p] = clean_k
                if p == "gemini": state["gemini_key"] = clean_k
                if p == "groq": state["groq_key"] = clean_k
                if p == "custom": state["custom_key"] = clean_k
            if model:
                state["active_model"] = str(model).strip()
            return True
        return f"[Aira AI Notice]: Unknown provider '{provider}'. Choose from: {list(WORLD_AI_PROVIDERS.keys())}"

    def set_endpoint(url, key=None, model=None):
        state["custom_endpoint"] = str(url).strip()
        state["active_provider"] = "custom"
        if key:
            state["custom_key"] = str(key).strip()
        if model:
            state["active_model"] = str(model).strip()
        return True

    def set_model(model):
        state["active_model"] = str(model).strip()
        return True

    def set_system(system_prompt):
        state["system_prompt"] = str(system_prompt)
        return True

    def list_providers():
        return {k: {"name": v["name"], "default_model": v["default_model"]} for k, v in WORLD_AI_PROVIDERS.items()}

    def _call_gemini(prompt, model, key, system):
        clean_key = str(key).strip()
        if not clean_key or clean_key.lower() in ("api key", "your_api_key", "your_key", "key"):
            return "[Aira AI Error]: Invalid or missing Gemini API key. Pass via ai.set_key('KEY') or set GEMINI_API_KEY."

        gemini_model = model or "gemini-1.5-flash"
        url = f"https://generativelanguage.googleapis.com/v1beta/models/{gemini_model}:generateContent?key={clean_key}"
        req_obj = {"contents": [{"parts": [{"text": str(prompt)}]}]}
        if system:
            req_obj["system_instruction"] = {"parts": [{"text": str(system)}]}
        payload = json.dumps(req_obj).encode("utf-8")
        req = urllib.request.Request(url, data=payload, headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=30) as resp:
                data = json.loads(resp.read().decode("utf-8"))
                return data["candidates"][0]["content"]["parts"][0]["text"]
        except urllib.error.HTTPError as http_err:
            try:
                err_body = json.loads(http_err.read().decode("utf-8"))
                msg = err_body.get("error", {}).get("message", str(http_err))
                return f"[Aira AI Error]: HTTP {http_err.code} - {msg}"
            except Exception:
                return f"[Aira AI Error]: HTTP {http_err.code}: {http_err.reason}"
        except Exception as e:
            return f"[Aira AI Error]: {e}"

    def _call_anthropic(key, prompt, model, system):
        if not key:
            return "[Aira Anthropic Error]: No API key configured. Call ai.set_key('KEY', 'anthropic') or set ANTHROPIC_API_KEY."
        url = "https://api.anthropic.com/v1/messages"
        req_body = {
            "model": model or "claude-3-5-sonnet-20241022",
            "max_tokens": 1024,
            "messages": [{"role": "user", "content": str(prompt)}]
        }
        if system:
            req_body["system"] = str(system)
        payload = json.dumps(req_body).encode("utf-8")
        headers = {
            "Content-Type": "application/json",
            "x-api-key": str(key).strip(),
            "anthropic-version": "2023-06-01",
            "User-Agent": "Mozilla/5.0 (AiraLang/1.4.1)"
        }
        req = urllib.request.Request(url, data=payload, headers=headers)
        try:
            with urllib.request.urlopen(req, timeout=35) as resp:
                data = json.loads(resp.read().decode("utf-8"))
                return data["content"][0]["text"]
        except urllib.error.HTTPError as err:
            try:
                err_body = json.loads(err.read().decode("utf-8"))
                msg = err_body.get("error", {}).get("message", str(err))
                return f"[Aira Anthropic Error]: HTTP {err.code} - {msg}"
            except Exception:
                return f"[Aira Anthropic Error]: HTTP {err.code}: {err.reason}"
        except Exception as e:
            return f"[Aira Anthropic Error]: {e}"

    def _call_openai_compat(url, key, prompt, model, system, extra_headers=None):
        messages = []
        if system:
            messages.append({"role": "system", "content": str(system)})
        messages.append({"role": "user", "content": str(prompt)})

        payload = json.dumps({
            "model": model,
            "messages": messages
        }).encode("utf-8")

        headers = {
            "Content-Type": "application/json",
            "User-Agent": "Mozilla/5.0 (AiraLang/1.4.1)"
        }
        if key:
            headers["Authorization"] = f"Bearer {key}"
        if extra_headers and isinstance(extra_headers, dict):
            headers.update(extra_headers)

        req = urllib.request.Request(url, data=payload, headers=headers)
        try:
            with urllib.request.urlopen(req, timeout=35) as resp:
                data = json.loads(resp.read().decode("utf-8"))
                return data["choices"][0]["message"]["content"]
        except urllib.error.HTTPError as err:
            try:
                err_body = json.loads(err.read().decode("utf-8"))
                msg = err_body.get("error", {}).get("message", str(err))
                return f"[Aira AI Error]: HTTP {err.code} - {msg}"
            except Exception:
                return f"[Aira AI Error]: HTTP {err.code}: {err.reason}"
        except Exception as e:
            return f"[Aira AI Error]: {e}"

    def groq(prompt, model=None, key=None, system=None):
        api_key = key or state["keys"].get("groq") or state.get("groq_key")
        if not api_key:
            return f"[Aira Groq Error]: No GROQ API key provided. Set GROQ_API_KEY or call ai.set_key('key', 'groq')."

        sys_content = system if system is not None else state.get("system_prompt", AIRA_DEFAULT_SYSTEM_PROMPT)
        models_to_try = [model] if model else ["openai/gpt-oss-120b", "openai/gpt-oss-20b", "qwen/qwen3.8-27b", "groq/compound"]
        url = "https://api.groq.com/openai/v1/chat/completions"

        for m in models_to_try:
            if not m:
                continue
            res = _call_openai_compat(url, api_key, prompt, m, sys_content)
            if not str(res).startswith("[Aira AI Error]: HTTP 404") or len(models_to_try) == 1:
                return res
        return "[Aira Groq Error]: All candidate models failed."

    def ask(prompt, model=None, key=None, system=None, provider=None):
        sys_to_use = system if system is not None else state.get("system_prompt", AIRA_DEFAULT_SYSTEM_PROMPT)
        target_provider = str(provider).lower() if provider else state.get("active_provider", "gemini")

        # Key inspection for auto-routing
        passed_key = str(key).strip() if key else ""
        if passed_key.startswith("gsk_"): target_provider = "groq"
        elif passed_key.startswith("sk-or-"): target_provider = "openrouter"
        elif passed_key.startswith("sk-ant-"): target_provider = "anthropic"
        elif passed_key.startswith("pplx-"): target_provider = "perplexity"
        elif passed_key.startswith("csk-"): target_provider = "cerebras"

        prov_info = WORLD_AI_PROVIDERS.get(target_provider, {})
        chosen_model = model or state.get("active_model") or prov_info.get("default_model", "")
        chosen_key = passed_key or state["keys"].get(target_provider) or state.get("custom_key", "")

        if not chosen_key and target_provider != "ollama":
            if prompt_developer_for_ai_key():
                target_provider = state.get("active_provider", target_provider)
                prov_info = WORLD_AI_PROVIDERS.get(target_provider, {})
                chosen_model = model or state.get("active_model") or prov_info.get("default_model", "")
                chosen_key = state["keys"].get(target_provider) or ""

        # 1. Custom Endpoint
        if target_provider == "custom" or state.get("custom_endpoint"):
            custom_url = state.get("custom_endpoint")
            return _call_openai_compat(custom_url, chosen_key, prompt, chosen_model or "default", sys_to_use)

        # 2. Gemini
        if target_provider == "gemini":
            k = chosen_key or state.get("gemini_key", "")
            if not k and state.get("groq_key"):
                return groq(prompt, model=chosen_model, system=sys_to_use)
            return _call_gemini(prompt, chosen_model or "gemini-1.5-flash", k, sys_to_use)

        # 3. Anthropic
        if target_provider == "anthropic":
            return _call_anthropic(chosen_key, prompt, chosen_model, sys_to_use)

        # 4. Groq special handling (fallback models)
        if target_provider == "groq" and not model:
            return groq(prompt, model=chosen_model, key=chosen_key, system=sys_to_use)

        # 5. Generic OpenAI-Compatible Providers (OpenAI, DeepSeek, OpenRouter, Mistral, Ollama, Together, Perplexity, Cerebras)
        if target_provider in WORLD_AI_PROVIDERS:
            endpoint_url = prov_info.get("url")
            if target_provider == "ollama":
                # Ollama is local, doesn't mandate API key
                return _call_openai_compat(endpoint_url, "", prompt, chosen_model or "llama3.2", sys_to_use)
            if not chosen_key:
                return f"[Aira AI Error]: No API key configured for provider '{target_provider}'. Call ai.set_key('KEY', '{target_provider}') or set {prov_info.get('env_var')}."
            return _call_openai_compat(endpoint_url, chosen_key, prompt, chosen_model, sys_to_use)

        # Fallback to Gemini
        return _call_gemini(prompt, "gemini-1.5-flash", state.get("gemini_key", ""), sys_to_use)

    def chat(messages, model=None, key=None, provider=None):
        last_msg = messages[-1]["content"] if isinstance(messages, list) and messages else str(messages)
        return ask(last_msg, model=model, key=key, provider=provider)

    def summarize(text, max_words=100, provider=None):
        prompt = f"Summarize the following text concisely in under {max_words} words:\n\n{text}"
        return ask(prompt, provider=provider)

    # Provider-specific direct shortcuts
    return AiraModule("ai", {
        "ask": ask,
        "chat": chat,
        "summarize": summarize,
        "set_key": set_key,
        "get_key": get_key,
        "set_provider": set_provider,
        "set_endpoint": set_endpoint,
        "set_model": set_model,
        "set_system": set_system,
        "system": set_system,
        "providers": list_providers,
        "groq": groq,
        "argon": lambda prompt, model="gemini-4-argon", key=None: ask(prompt, model=model, key=key, provider="gemini"),
        "gemini": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="gemini"),
        "openai": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="openai"),
        "deepseek": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="deepseek"),
        "openrouter": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="openrouter"),
        "anthropic": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="anthropic"),
        "mistral": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="mistral"),
        "ollama": lambda prompt, model=None: ask(prompt, model=model, provider="ollama"),
        "together": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="together"),
        "perplexity": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="perplexity"),
        "cerebras": lambda prompt, model=None, key=None: ask(prompt, model=model, key=key, provider="cerebras")
    })

def create_sec_module():
    COMMON_SERVICES = {
        21: "FTP", 22: "SSH", 23: "Telnet", 25: "SMTP", 53: "DNS",
        80: "HTTP", 110: "POP3", 111: "RPCBind", 135: "RPC", 139: "NetBIOS",
        143: "IMAP", 443: "HTTPS", 445: "SMB", 993: "IMAPS", 995: "POP3S",
        1433: "MSSQL", 1521: "Oracle", 3306: "MySQL", 3389: "RDP",
        5432: "PostgreSQL", 5900: "VNC", 6379: "Redis", 8000: "HTTP-Alt",
        8080: "HTTP-Proxy", 8443: "HTTPS-Alt", 8888: "HTTP-Alt", 9000: "SonarQube",
        27017: "MongoDB"
    }

    def scan_port(host, port, timeout=1.0):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(float(timeout))
            res = s.connect_ex((str(host), int(port)))
            s.close()
            return res == 0
        except Exception:
            return False

    def scan_ports(host, ports=None, threads=15, timeout=1.0):
        if ports is None:
            ports = [21, 22, 23, 25, 53, 80, 110, 143, 443, 445, 1433, 1521, 3306, 3389, 5432, 6379, 8000, 8080, 8443, 8888]
        elif isinstance(ports, range):
            ports = list(ports)
        elif not isinstance(ports, list):
            ports = [int(ports)]

        open_ports = []
        def _check(p):
            if scan_port(host, p, timeout=timeout):
                svc = COMMON_SERVICES.get(p, "Unknown")
                return {"port": p, "service": svc, "state": "open"}
            return None

        with ThreadPoolExecutor(max_workers=int(threads)) as executor:
            results = executor.map(_check, ports)
            for r in results:
                if r is not None:
                    open_ports.append(r)
        return open_ports

    def banner(host, port, timeout=2.0):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(float(timeout))
            s.connect((str(host), int(port)))
            if int(port) in (80, 8080, 8000, 8888):
                s.sendall(b"HEAD / HTTP/1.0\r\n\r\n")
            else:
                s.sendall(b"\r\n")
            data = s.recv(1024).decode('utf-8', errors='ignore').strip()
            s.close()
            return data
        except Exception as e:
            return f"Error: {e}"

    def audit_headers(url):
        if not str(url).startswith("http://") and not str(url).startswith("https://"):
            url = "https://" + str(url)
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "AiraLang-SecEngine/1.4.0"})
            with urllib.request.urlopen(req, timeout=10) as resp:
                headers = dict(resp.headers)

            important_headers = [
                "Strict-Transport-Security",
                "Content-Security-Policy",
                "X-Frame-Options",
                "X-Content-Type-Options",
                "Referrer-Policy",
                "Permissions-Policy"
            ]

            present = []
            missing = []
            header_keys_lower = {k.lower(): k for k in headers.keys()}

            for h in important_headers:
                if h.lower() in header_keys_lower:
                    present.append(h)
                else:
                    missing.append(h)

            score_ratio = len(present) / len(important_headers)
            if score_ratio >= 0.8: grade = "A"
            elif score_ratio >= 0.6: grade = "B"
            elif score_ratio >= 0.4: grade = "C"
            elif score_ratio >= 0.2: grade = "D"
            else: grade = "F"

            return {
                "url": url,
                "grade": grade,
                "present": present,
                "missing": missing,
                "server": headers.get("Server", headers.get("server", "Hidden/Unknown")),
                "headers": headers
            }
        except Exception as e:
            return {"url": url, "error": str(e)}

    def subdomains(domain, timeout=8.0):
        try:
            clean_dom = str(domain).replace("https://", "").replace("http://", "").split("/")[0]
            url = f"https://crt.sh/?q=%.{urllib.parse.quote(clean_dom)}&output=json"
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
            with urllib.request.urlopen(req, timeout=float(timeout)) as resp:
                data = json.loads(resp.read().decode('utf-8'))
            subs = set()
            for entry in data:
                name_val = entry.get("name_value", "")
                for sub in name_val.split("\n"):
                    sub = sub.strip().lower()
                    if sub and "*" not in sub:
                        subs.add(sub)
            return sorted(list(subs))
        except Exception as e:
            return [f"Error fetching subdomains: {e}"]

    def hash_identify(hash_str):
        h = str(hash_str).strip()
        l = len(h)
        is_hex = all(c in "0123456789abcdefABCDEF" for c in h)

        if h.startswith("$2a$") or h.startswith("$2b$") or h.startswith("$2y$"):
            return "Bcrypt"
        if h.startswith("$6$"):
            return "SHA-512 Crypt"
        if h.startswith("$1$"):
            return "MD5 Crypt"

        if is_hex:
            if l == 32: return "MD5 / NTLM"
            elif l == 40: return "SHA-1"
            elif l == 56: return "SHA-224"
            elif l == 64: return "SHA-256"
            elif l == 96: return "SHA-384"
            elif l == 128: return "SHA-512"

        return "Unknown Hash Format"

    def crack_md5(target_hash, wordlist):
        target = str(target_hash).lower().strip()
        words = []
        if isinstance(wordlist, list):
            words = wordlist
        elif isinstance(wordlist, str):
            if os.path.exists(wordlist):
                with open(wordlist, "r", encoding="utf-8", errors="ignore") as f:
                    words = [line.strip() for line in f]
            else:
                words = [wordlist]

        for w in words:
            if hashlib.md5(w.encode('utf-8')).hexdigest() == target:
                return w
        return None

    def resolve(host):
        try:
            return socket.gethostbyname(str(host))
        except Exception as e:
            return str(e)

    def reverse_dns(ip):
        try:
            return socket.gethostbyaddr(str(ip))[0]
        except Exception as e:
            return str(e)

    def ssl_check(domain, port=443, timeout=5.0):
        clean_dom = str(domain).replace("https://", "").replace("http://", "").split("/")[0].split(":")[0]
        try:
            ctx = ssl.create_default_context()
            with socket.create_connection((clean_dom, int(port)), timeout=float(timeout)) as sock:
                with ctx.wrap_socket(sock, server_hostname=clean_dom) as ssock:
                    cert = ssock.getpeercert()
                    cipher = ssock.cipher()
                    version = ssock.version()
                    not_after = cert.get('notAfter')
                    expiry = datetime.datetime.strptime(not_after, "%b %d %H:%M:%S %Y %Z") if not_after else None
                    days_left = (expiry - datetime.datetime.utcnow()).days if expiry else None
                    issuer_dict = dict(x[0] for x in cert.get('issuer', []))
                    subject_dict = dict(x[0] for x in cert.get('subject', []))
                    return {
                        "domain": clean_dom,
                        "valid": days_left > 0 if days_left is not None else False,
                        "days_left": days_left,
                        "expires_on": not_after,
                        "tls_version": version,
                        "cipher": cipher[0] if cipher else None,
                        "issuer": issuer_dict.get('commonName', issuer_dict.get('organizationName', 'Unknown')),
                        "subject": subject_dict.get('commonName', 'Unknown')
                    }
        except Exception as e:
            return {"domain": clean_dom, "valid": False, "error": str(e)}

    def dns_lookup(domain):
        clean_dom = str(domain).replace("https://", "").replace("http://", "").split("/")[0]
        try:
            addr_info = socket.getaddrinfo(clean_dom, None)
            ips = sorted(list(set(item[4][0] for item in addr_info)))
            return {"domain": clean_dom, "ips": ips, "count": len(ips)}
        except Exception as e:
            return {"domain": clean_dom, "error": str(e), "ips": []}

    return AiraModule("sec", {
        "scan_port": scan_port,
        "scan_ports": scan_ports,
        "scan": scan_ports,
        "banner": banner,
        "audit_headers": audit_headers,
        "subdomains": subdomains,
        "hash_identify": hash_identify,
        "crack_md5": crack_md5,
        "resolve": resolve,
        "reverse_dns": reverse_dns,
        "ssl_check": ssl_check,
        "ssl": ssl_check,
        "dns_lookup": dns_lookup,
        "dns": dns_lookup
    })


def create_thread_module():
    def spawn(fn, args=None):
        if args is None:
            args = []
        elif not isinstance(args, (list, tuple)):
            args = [args]

        def _runner():
            if callable(fn):
                fn(*args)

        t = threading.Thread(target=_runner, daemon=True)
        t.start()
        return t

    def join(t, timeout=None):
        if hasattr(t, "join"):
            t.join(timeout=float(timeout) if timeout is not None else None)
            return True
        return False

    def sleep(seconds):
        time.sleep(float(seconds))
        return True

    def pool(fn, items, max_workers=5):
        if not isinstance(items, (list, tuple)):
            items = list(items)

        def _worker(item):
            if callable(fn):
                return fn(item)
            return None

        with ThreadPoolExecutor(max_workers=int(max_workers)) as executor:
            return list(executor.map(_worker, items))

    return AiraModule("thread", {
        "spawn": spawn,
        "start": spawn,
        "join": join,
        "sleep": sleep,
        "pool": pool
    })

AIRA_LOVER_CHECKER_SYSTEM_PROMPT = (
    "You are Aira, the official AI engine of AiraLang. You are the Lover Accept Checker AI. "
    "Your duty is to analyze responses to romantic proposals. Determine whether the response indicates acceptance, agreement, romantic interest, or affection "
    "(including in Manglish, Malayalam, Hindi, slang, or subtle hints like 'mmh', 'aah', 'athe', 'undu', 'ath pinne parayano', 'love you', 'sure', 'yes'). "
    "If the response accepts or shows affection, output strictly YES. If rejected, avoided, or negative, output strictly NO. "
    "Answer strictly with ONLY one word: YES or NO."
)

AIRA_CELEBRATION_WISHES = [
    "✨ [Aira]: Awww, Proposal Accepted! 💍💖 Aira wishes you both a lifetime of unconditional love, cute moments & endless happiness! 🫶✨",
    "✨ [Aira]: Omg, it's a YES! 🎉💑 Aira wishes the sweetest couple a beautiful journey filled with love and togetherness! 💖✨",
    "✨ [Aira]: Woohoo, Heart Connected! 💘✨ Aira sends all the love and blessings to both of you for a magical love story! 🥂❤️",
    "✨ [Aira]: Proposal Accepted! 🫶 Aira wishes you both endless laughs, late-night talks, and forever love! 💖✨"
]

def create_proposal_module():
    state = _GLOBAL_AI_STATE

    def set_key(key):
        clean = str(key).strip()
        if clean.startswith("gsk_"):
            state["groq_key"] = clean
            state["active_provider"] = "groq"
        else:
            state["gemini_key"] = clean
            state["active_provider"] = "gemini"
        return True

    def set_system(sys_prompt):
        state["proposal_system_prompt"] = str(sys_prompt)
        return True

    def set_wish(custom_wish):
        state["proposal_custom_wish"] = str(custom_wish)
        return True

    def wish(custom_message=None):
        if custom_message:
            print(f"\n✨ [Aira]: {custom_message} 💖✨\n")
        else:
            w = state.get("proposal_custom_wish") or random.choice(AIRA_CELEBRATION_WISHES)
            print(f"\n{w}\n")
        return True

    def ask(prompt="Do You Love Me : ", api_key=None, wish_enabled=True):
        if api_key:
            set_key(api_key)
        key = state["groq_key"] if state.get("active_provider") == "groq" else state["gemini_key"]

        accept_keywords = {
            "yes", "y", "yeah", "yep", "yup", "sure", "ok", "okay",
            "of course", "definitely", "always", "true", "1",
            "mmh", "mmm", "aah", "aa", "athe", "athaanu", "undu",
            "und", "pinne", "istam", "ishttam", "love you", "kollam",
            "haan", "ha", "zaroor", "si", "oui", "ja"
        }

        def _trigger_accept():
            if wish_enabled:
                w = state.get("proposal_custom_wish") or random.choice(AIRA_CELEBRATION_WISHES)
                print(f"\n{w}\n")
            return True

        while True:
            try:
                user_reply = input(str(prompt)).strip()
            except (EOFError, KeyboardInterrupt):
                print("\n[!] Proposal interaction terminated.")
                return False

            clean_reply = user_reply.lower()

            # Direct affirmative keyword match
            if clean_reply in accept_keywords:
                return _trigger_accept()

            # AI Lover Accept Checker semantic check if key is available
            if key and key != "api key":
                try:
                    ai_mod = create_ai_module()
                    sys_prompt = state.get("proposal_system_prompt", AIRA_LOVER_CHECKER_SYSTEM_PROMPT)
                    user_msg = (
                        f"The proposal was: '{prompt}'\n"
                        f"The user responded with: '{user_reply}'\n"
                        "Did they accept the proposal or show love/affection? Output strictly YES or NO."
                    )
                    ai_resp = ai_mod.get("ask")(user_msg, system=sys_prompt)
                    if isinstance(ai_resp, str) and "YES" in ai_resp.upper():
                        return _trigger_accept()
                except Exception:
                    pass

            # Simulated HTTP 500 Timeout loop
            print("\n[!] HTTP 500: Server Love Timeout Error!")
            print("[!] Request timed out: Target response did not evaluate to affirmative love.")
            print("[!] Retrying connection to heart server in 1s...\n")
            time.sleep(1)

    return AiraModule("proposal", {
        "ask": ask,
        "set_key": set_key,
        "api_key": set_key,
        "set_system": set_system,
        "system": set_system,
        "wish": wish,
        "set_wish": set_wish
    })

def create_android_module():
    DANGEROUS_PERMISSIONS = {
        "android.permission.READ_SMS": "Can read SMS messages and OTP verification codes",
        "android.permission.RECEIVE_SMS": "Can intercept incoming SMS messages in real-time",
        "android.permission.SEND_SMS": "Can send premium SMS messages without user consent",
        "android.permission.ACCESS_FINE_LOCATION": "Accesses precise GPS physical coordinates",
        "android.permission.ACCESS_COARSE_LOCATION": "Accesses network/cell-tower based location",
        "android.permission.ACCESS_BACKGROUND_LOCATION": "Continuously tracks physical location in background",
        "android.permission.RECORD_AUDIO": "Can access microphone and record ambient audio",
        "android.permission.CAMERA": "Can access camera to capture covert photos or video",
        "android.permission.READ_CONTACTS": "Can harvest complete address book and contacts",
        "android.permission.WRITE_CONTACTS": "Can modify or delete address book entries",
        "android.permission.READ_CALL_LOG": "Can read private phone call logs and history",
        "android.permission.WRITE_CALL_LOG": "Can alter or delete telephone call history",
        "android.permission.READ_PHONE_STATE": "Can read device IMEI, IMSI, and SIM serials",
        "android.permission.READ_EXTERNAL_STORAGE": "Can read media, documents, and downloads on storage",
        "android.permission.WRITE_EXTERNAL_STORAGE": "Can write or overwrite arbitrary storage files",
        "android.permission.MANAGE_EXTERNAL_STORAGE": "Full filesystem access bypassing Android scoped storage",
        "android.permission.SYSTEM_ALERT_WINDOW": "Can draw floating window overlays (Tapjacking risk)",
        "android.permission.REQUEST_INSTALL_PACKAGES": "Can silently trigger or sideload unverified APKs",
        "android.permission.USE_BIOMETRIC": "Requests biometric fingerprint or face authentication",
        "android.permission.BIND_ACCESSIBILITY_SERVICE": "High risk: Can observe screen and simulate user inputs"
    }

    def _getprop(prop):
        try:
            p = subprocess.run(["/system/bin/getprop", str(prop)], capture_output=True, text=True, timeout=2)
            return p.stdout.strip()
        except Exception:
            return ""

    def get_prop(prop_name):
        return _getprop(prop_name)

    def device_info():
        sdk_str = _getprop("ro.build.version.sdk")
        return {
            "brand": _getprop("ro.product.brand") or "Android",
            "model": _getprop("ro.product.model") or "Generic Device",
            "manufacturer": _getprop("ro.product.manufacturer") or "Unknown",
            "android_version": _getprop("ro.build.version.release") or "Unknown",
            "sdk_version": int(sdk_str) if sdk_str.isdigit() else 0,
            "arch": _getprop("ro.product.cpu.abi") or (os.uname().machine if hasattr(os, "uname") else "unknown"),
            "build_type": _getprop("ro.build.type") or "user"
        }

    def is_rooted():
        su_paths = [
            "/system/bin/su", "/system/xbin/su", "/sbin/su",
            "/data/local/xbin/su", "/data/local/bin/su", "/system/sd/xbin/su",
            "/su/bin/su", "/magisk/.core/bin/su", "/data/adb/magisk"
        ]
        for path in su_paths:
            if os.path.exists(path):
                return True
        try:
            p = subprocess.run(["which", "su"], capture_output=True, text=True, timeout=2)
            if p.returncode == 0 and p.stdout.strip():
                return True
        except Exception:
            pass
        return False

    def audit_device():
        findings = []
        recommendations = []
        score = 100

        rooted = is_rooted()
        if rooted:
            score -= 40
            findings.append("CRITICAL: Root binary or Magisk artifact detected on device.")
            recommendations.append("Unroot device or configure Magisk DenyList to protect sensitive data.")

        flash_locked = _getprop("ro.boot.flash.locked")
        bootloader_locked = (flash_locked == "1")
        if not bootloader_locked and flash_locked:
            score -= 25
            findings.append("HIGH: Bootloader is UNLOCKED. Physical device integrity verification is disabled.")
            recommendations.append("Lock bootloader to prevent unauthorized physical firmware attacks.")

        avb_state = _getprop("ro.boot.verifiedbootstate") or "unknown"
        if avb_state.lower() not in ("green", "unknown", ""):
            score -= 20
            findings.append(f"HIGH: Android Verified Boot (AVB) state is {avb_state.upper()} (Tampered system).")
            recommendations.append("Reflash official OEM stock firmware to restore Verified Boot trust chain.")

        debuggable = (_getprop("ro.debuggable") == "1")
        if debuggable:
            score -= 15
            findings.append("MEDIUM: Android OS is built with ro.debuggable=1 (Debug ROM).")
            recommendations.append("Switch to official release-keys user build.")

        score = max(0, min(100, score))
        if score >= 85:
            posture = "SECURE"
        elif score >= 60:
            posture = "MODERATE_RISK"
        else:
            posture = "HIGH_RISK"

        return {
            "posture": posture,
            "security_score": score,
            "is_rooted": rooted,
            "bootloader_locked": bootloader_locked,
            "verified_boot": avb_state,
            "is_debuggable_os": debuggable,
            "findings": findings,
            "recommendations": recommendations
        }

    def _extract_axml_strings(data):
        strings = []
        try:
            offset = 8
            chunk_type, header_size, chunk_size = struct.unpack("<HHI", data[offset:offset+8])
            if chunk_type == 0x0001:
                str_count, style_count, flags, str_start, style_start = struct.unpack("<IIIII", data[offset+8:offset+28])
                is_utf8 = bool(flags & (1 << 8))
                offsets_start = offset + header_size
                offsets = [struct.unpack("<I", data[offsets_start + i*4 : offsets_start + (i+1)*4])[0] for i in range(str_count)]
                base = offset + str_start
                for off in offsets:
                    cur = base + off
                    if is_utf8:
                        while cur < len(data) and data[cur] & 0x80:
                            cur += 1
                        cur += 1
                        end = data.find(b"\x00", cur)
                        if end != -1:
                            strings.append(data[cur:end].decode("utf-8", errors="ignore"))
                    else:
                        u16_len = struct.unpack("<H", data[cur:cur+2])[0]
                        cur += 2
                        if u16_len & 0x8000:
                            u16_len = ((u16_len & 0x7fff) << 16) | struct.unpack("<H", data[cur:cur+2])[0]
                            cur += 2
                        str_bytes = data[cur:cur + u16_len*2]
                        strings.append(str_bytes.decode("utf-16le", errors="ignore"))
        except Exception:
            pass
        return strings

    def audit_apk(apk_path):
        target = str(apk_path).strip()
        if not os.path.exists(target):
            return {
                "error": f"File not found: {target}",
                "apk_path": target,
                "package_name": "None",
                "risk_level": "NOT_FOUND",
                "risk_score": 0,
                "is_signed": False,
                "debug_certificate": False,
                "flags": {"debuggable": False, "allow_backup": False, "cleartext_traffic": False, "test_only": False},
                "total_permissions_count": 0,
                "dangerous_permissions": {},
                "hardcoded_secrets": [],
                "insecure_http_urls": [],
                "findings": [f"File not found: {target}"]
            }
        if not zipfile.is_zipfile(target):
            return {
                "error": f"Invalid APK or ZIP format: {target}",
                "apk_path": target,
                "package_name": "None",
                "risk_level": "INVALID_FORMAT",
                "risk_score": 0,
                "is_signed": False,
                "debug_certificate": False,
                "flags": {"debuggable": False, "allow_backup": False, "cleartext_traffic": False, "test_only": False},
                "total_permissions_count": 0,
                "dangerous_permissions": {},
                "hardcoded_secrets": [],
                "insecure_http_urls": [],
                "findings": [f"Invalid APK or ZIP format: {target}"]
            }

        findings = []
        risk_score = 0
        package_name = "Unknown"
        all_perms = []
        dangerous_perms = {}
        flags = {
            "debuggable": False,
            "allow_backup": False,
            "cleartext_traffic": False,
            "test_only": False
        }
        hardcoded_secrets = []
        insecure_http_urls = set()

        try:
            with zipfile.ZipFile(target, "r") as z:
                names = z.namelist()

                # 1. Parse AndroidManifest.xml
                if "AndroidManifest.xml" in names:
                    m_data = z.read("AndroidManifest.xml")
                    m_strings = _extract_axml_strings(m_data)

                    for s in m_strings:
                        if "android.permission." in s and s not in all_perms:
                            all_perms.append(s)
                            if s in DANGEROUS_PERMISSIONS:
                                dangerous_perms[s] = DANGEROUS_PERMISSIONS[s]
                        if package_name == "Unknown" and (s.startswith("com.") or s.startswith("org.") or s.startswith("net.") or s.startswith("io.")):
                            if "." in s and " " not in s and len(s) < 64:
                                package_name = s

                    m_blob = str(m_strings)
                    if "debuggable" in m_blob:
                        flags["debuggable"] = True
                        findings.append("CRITICAL: android:debuggable is ENABLED in manifest (Allows debugger & memory attach)!")
                        risk_score += 35
                    if "allowBackup" in m_blob:
                        flags["allow_backup"] = True
                        findings.append("MEDIUM: android:allowBackup is active (App data extractable via ADB backup).")
                        risk_score += 15
                    if "usesCleartextTraffic" in m_blob:
                        flags["cleartext_traffic"] = True
                        findings.append("MEDIUM: usesCleartextTraffic is active (Unencrypted HTTP allowed).")
                        risk_score += 15

                # 2. Inspect DEX bytecode for hardcoded keys & plaintext HTTP
                dex_files = [f for f in names if f.endswith(".dex")][:3]
                for df in dex_files:
                    try:
                        dex_content = z.read(df)
                        for gk in set(re.findall(rb"AIza[0-9A-Za-z-_]{35}", dex_content)):
                            hardcoded_secrets.append({"type": "Google API Key", "key": gk.decode("ascii", errors="ignore")})
                        for ak in set(re.findall(rb"AKIA[0-9A-Z]{16}", dex_content)):
                            hardcoded_secrets.append({"type": "AWS Access Key", "key": ak.decode("ascii", errors="ignore")})
                        http_urls = re.findall(rb"http://[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}(?:/[^\s\"\'<>]*)?", dex_content)
                        for u in http_urls[:10]:
                            insecure_http_urls.add(u.decode("ascii", errors="ignore"))
                    except Exception:
                        pass

                # 3. Inspect META-INF Signatures
                cert_files = [f for f in names if f.startswith("META-INF/") and f.endswith((".RSA", ".DSA", ".EC"))]
                is_signed = len(cert_files) > 0
                debug_cert = False
                for cf in cert_files:
                    try:
                        cb = z.read(cf)
                        if b"Android Debug" in cb or b"CN=Android Debug" in cb:
                            debug_cert = True
                            findings.append("CRITICAL: APK signed with insecure Android Debug Keystore!")
                            risk_score += 40
                    except Exception:
                        pass

        except Exception as e:
            return {"error": f"Failed auditing APK: {e}"}

        if len(dangerous_perms) > 5:
            risk_score += 20
            findings.append(f"HIGH: Requests {len(dangerous_perms)} high-risk dangerous permissions!")
        elif len(dangerous_perms) > 0:
            risk_score += 10

        if hardcoded_secrets:
            risk_score += 25
            findings.append(f"HIGH: Identified {len(hardcoded_secrets)} potential hardcoded API/Cloud keys in DEX bytecode!")

        if risk_score >= 50:
            risk_level = "CRITICAL"
        elif risk_score >= 30:
            risk_level = "HIGH"
        elif risk_score >= 15:
            risk_level = "MEDIUM"
        else:
            risk_level = "LOW"

        return {
            "apk_path": target,
            "package_name": package_name,
            "risk_level": risk_level,
            "risk_score": risk_score,
            "is_signed": is_signed,
            "debug_certificate": debug_cert,
            "flags": flags,
            "total_permissions_count": len(all_perms),
            "dangerous_permissions": dangerous_perms,
            "hardcoded_secrets": hardcoded_secrets,
            "insecure_http_urls": list(insecure_http_urls)[:10],
            "findings": findings
        }

    def audit_package(pkg_name):
        pkg = str(pkg_name).strip()
        try:
            p = subprocess.run(["pm", "path", pkg], capture_output=True, text=True, timeout=3)
            out = p.stdout.strip()
            if "package:" in out:
                apk_path = out.split("package:")[1].split("\n")[0].strip()
                if os.path.exists(apk_path):
                    return audit_apk(apk_path)
        except Exception:
            pass

        for base_dir in ["/data/app", "/system/app", "/system/priv-app"]:
            if os.path.exists(base_dir):
                try:
                    for entry in os.listdir(base_dir):
                        if pkg in entry:
                            full = os.path.join(base_dir, entry)
                            if os.path.isfile(full) and full.endswith(".apk"):
                                return audit_apk(full)
                            elif os.path.isdir(full):
                                for sub in os.listdir(full):
                                    if sub.endswith(".apk"):
                                        return audit_apk(os.path.join(full, sub))
                except Exception:
                    pass

        return {
            "package": pkg,
            "status": "not_found",
            "message": f"Could not locate APK for '{pkg}'. On unrooted Android, pass direct APK file path to android.audit_apk(path)."
        }

    return AiraModule("android", {
        "device_info": device_info,
        "info": device_info,
        "audit_device": audit_device,
        "audit": audit_device,
        "is_rooted": is_rooted,
        "check_root": is_rooted,
        "audit_apk": audit_apk,
        "audit_package": audit_package,
        "get_prop": get_prop
    })

def create_system_module():
    def get_gpu():
        try:
            p = subprocess.run(
                ["nvidia-smi", "--query-gpu=name,memory.total,memory.free,driver_version,temperature.gpu", "--format=csv,noheader,nounits"],
                capture_output=True, text=True, timeout=3
            )
            if p.returncode == 0 and p.stdout.strip():
                parts = [x.strip() for x in p.stdout.strip().split(",")]
                if len(parts) >= 5:
                    return {
                        "name": parts[0],
                        "total_vram_mb": int(parts[1]),
                        "free_vram_mb": int(parts[2]),
                        "driver_version": parts[3],
                        "temperature_c": int(parts[4]),
                        "status": "active"
                    }
        except Exception:
            pass
        return {
            "name": "Standard / Integrated Graphics",
            "status": "integrated_or_unavailable",
            "total_vram_mb": 0
        }

    def get_battery():
        if sys.platform == "win32":
            try:
                import ctypes
                class SYSTEM_POWER_STATUS(ctypes.Structure):
                    _fields_ = [
                        ('ac', ctypes.c_byte),
                        ('bf', ctypes.c_byte),
                        ('pct', ctypes.c_byte),
                        ('r', ctypes.c_byte),
                        ('lt', ctypes.c_ulong),
                        ('flt', ctypes.c_ulong),
                    ]
                s = SYSTEM_POWER_STATUS()
                if ctypes.windll.kernel32.GetSystemPowerStatus(ctypes.byref(s)):
                    return {
                        "percentage": int(s.pct) if s.pct <= 100 else 100,
                        "is_charging": bool(s.ac == 1),
                        "power_source": "AC (Charger Plugged In)" if s.ac == 1 else "Battery"
                    }
            except Exception:
                pass
        for path in ["/sys/class/power_supply/battery", "/sys/class/power_supply/BAT0"]:
            cap_file = os.path.join(path, "capacity")
            status_file = os.path.join(path, "status")
            if os.path.exists(cap_file):
                try:
                    with open(cap_file) as f:
                        cap = int(f.read().strip())
                    stat = "Discharging"
                    if os.path.exists(status_file):
                        with open(status_file) as f:
                            stat = f.read().strip()
                    return {
                        "percentage": cap,
                        "is_charging": stat.lower() == "charging",
                        "power_source": stat
                    }
                except Exception:
                    pass
        return {"percentage": 100, "is_charging": True, "power_source": "AC / Desktop"}

    def get_ram():
        if sys.platform == "win32":
            try:
                import ctypes
                class MEMORYSTATUSEX(ctypes.Structure):
                    _fields_ = [
                        ('dwLength', ctypes.c_ulong),
                        ('dwMemoryLoad', ctypes.c_ulong),
                        ('ullTotalPhys', ctypes.c_ulonglong),
                        ('ullAvailPhys', ctypes.c_ulonglong),
                        ('ullTotalPageFile', ctypes.c_ulonglong),
                        ('ullAvailPageFile', ctypes.c_ulonglong),
                        ('ullTotalVirtual', ctypes.c_ulonglong),
                        ('ullAvailVirtual', ctypes.c_ulonglong),
                        ('sullAvailExtendedVirtual', ctypes.c_ulonglong),
                    ]
                stat = MEMORYSTATUSEX()
                stat.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
                if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(stat)):
                    total_gb = round(stat.ullTotalPhys / (1024 ** 3), 2)
                    avail_gb = round(stat.ullAvailPhys / (1024 ** 3), 2)
                    used_gb = round(total_gb - avail_gb, 2)
                    return {
                        "total_gb": total_gb,
                        "available_gb": avail_gb,
                        "used_gb": used_gb,
                        "percent_used": int(stat.dwMemoryLoad)
                    }
            except Exception:
                pass
        try:
            with open("/proc/meminfo") as f:
                lines = f.readlines()
            mem = {}
            for l in lines:
                parts = l.split(":")
                if len(parts) == 2:
                    mem[parts[0].strip()] = int(parts[1].split()[0])
            total = round(mem.get("MemTotal", 0) / (1024 * 1024), 2)
            avail = round(mem.get("MemAvailable", 0) / (1024 * 1024), 2)
            return {"total_gb": total, "available_gb": avail, "used_gb": round(total - avail, 2), "percent_used": int(round(((total - avail) / total) * 100)) if total else 0}
        except Exception:
            return {"total_gb": 0, "available_gb": 0, "used_gb": 0, "percent_used": 0}

    def get_cpu():
        import multiprocessing
        cores = multiprocessing.cpu_count()
        cpu_name = "x86_64 Processor"
        if sys.platform == "win32":
            cpu_name = os.environ.get("PROCESSOR_IDENTIFIER", "x86_64 Processor")
        else:
            try:
                with open("/proc/cpuinfo") as f:
                    for line in f:
                        if "model name" in line:
                            cpu_name = line.split(":")[1].strip()
                            break
            except Exception:
                pass
        return {
            "cores": cores,
            "architecture": sys.platform,
            "name": cpu_name
        }

    def get_info():
        return {
            "platform": sys.platform,
            "cpu": get_cpu(),
            "ram": get_ram(),
            "gpu": get_gpu(),
            "battery": get_battery()
        }

    return AiraModule("system", {
        "info": get_info,
        "device_info": get_info,
        "gpu": get_gpu,
        "battery": get_battery,
        "ram": get_ram,
        "cpu": get_cpu
    })

def is_package_installed(pkg_name):
    config_dir = os.path.expanduser("~/.config/airalang")
    packages_file = os.path.join(config_dir, "installed_packages.json")
    if os.path.exists(packages_file):
        try:
            with open(packages_file, "r", encoding="utf-8") as f:
                pkgs = json.load(f)
                return pkg_name.lower() in pkgs
        except Exception:
            pass
    return False

def create_youtube_module():
    if not is_package_installed("youtube") and not is_package_installed("yt"):
        raise ImportError(
            "\033[1;31m[AiraLang Package Error]:\033[0m Module 'youtube' is not installed!\n"
            "\033[1;36m💡 Run:\033[0m aira install youtube"
        )
    try:
        import yt_dlp
    except ImportError:
        raise ImportError(
            "\033[1;31m[AiraLang Package Error]:\033[0m Backend engine 'yt-dlp' is missing.\n"
            "\033[1;36m💡 Run:\033[0m aira install youtube"
        )

    def get_video(url):
        url_str = str(url).strip()
        ydl_opts = {
            'quiet': True,
            'no_warnings': True,
            'extract_flat': True,
            'skip_download': True,
            'extractor_args': {'youtube': {'player_client': ['android', 'ios', 'web']}}
        }
        try:
            with yt_dlp.YoutubeDL(ydl_opts) as ydl:
                info = ydl.extract_info(url_str, download=False)
        except Exception as e:
            raise RuntimeError(f"Aira YouTube Error: Failed to fetch video info: {e}")

        title = info.get("title", "YouTube Video")
        duration = info.get("duration", 0)
        author = info.get("uploader", info.get("channel", "Unknown Channel"))
        views = info.get("view_count", 0)

        def get_stream(fmt="mp4"):
            fmt_str = str(fmt).lower().strip()

            def download(save_path=None):
                if save_path:
                    outtmpl = str(save_path).strip()
                else:
                    outtmpl = f"{title}.%(ext)s"

                parent_dir = os.path.dirname(outtmpl)
                if parent_dir:
                    os.makedirs(parent_dir, exist_ok=True)

                if fmt_str in ("mp3", "audio", "m4a"):
                    ydl_dl_opts = {
                        'format': 'bestaudio/best/140/251',
                        'outtmpl': outtmpl,
                        'quiet': False,
                        'no_warnings': True,
                        'extractor_args': {'youtube': {'player_client': ['android', 'ios', 'web']}}
                    }
                else:
                    ydl_dl_opts = {
                        'format': 'best[ext=mp4]/18/22/b/best',
                        'outtmpl': outtmpl,
                        'quiet': False,
                        'no_warnings': True,
                        'extractor_args': {'youtube': {'player_client': ['android', 'ios', 'web']}}
                    }

                try:
                    with yt_dlp.YoutubeDL(ydl_dl_opts) as dl:
                        dl.download([url_str])
                    return outtmpl
                except Exception as e:
                    raise RuntimeError(f"Aira YouTube Download Error: {e}")

            return AiraModule("youtube_stream", {
                "download": download,
                "format": fmt_str
            })

        return AiraModule("youtube_video", {
            "title": title,
            "duration": duration,
            "author": author,
            "uploader": author,
            "views": views,
            "url": url_str,
            "stream": get_stream
        })

    def quick_download(url, save_path=None, fmt="mp4"):
        video = get_video(url)
        st = video.get("stream")(fmt)
        return st.get("download")(save_path)

    return AiraModule("youtube", {
        "get": get_video,
        "video": get_video,
        "url": get_video,
        "download": quick_download
    })

def create_term_module():
    COLORS = {
        "black": "\033[30m", "red": "\033[31m", "green": "\033[32m", "yellow": "\033[33m",
        "blue": "\033[34m", "magenta": "\033[35m", "cyan": "\033[36m", "white": "\033[37m",
        "bright_black": "\033[90m", "bright_red": "\033[91m", "bright_green": "\033[92m",
        "bright_yellow": "\033[93m", "bright_blue": "\033[94m", "bright_magenta": "\033[95m",
        "bright_cyan": "\033[96m", "bright_white": "\033[97m",
        "bold": "\033[1m", "dim": "\033[2m", "italic": "\033[3m", "underline": "\033[4m",
        "reset": "\033[0m"
    }

    def color(col_name, text):
        c = COLORS.get(str(col_name).lower(), "")
        return f"{c}{text}\033[0m"

    def rainbow(text):
        rainbow_colors = ["\033[91m", "\033[93m", "\033[92m", "\033[96m", "\033[94m", "\033[95m"]
        res = []
        for i, ch in enumerate(str(text)):
            c = rainbow_colors[i % len(rainbow_colors)]
            res.append(f"{c}{ch}")
        res.append("\033[0m")
        return "".join(res)

    def box(title, content=None, color_name="cyan", width=50):
        w = max(int(width), len(str(title)) + 6)
        c = COLORS.get(str(color_name).lower(), "\033[36m")
        reset = "\033[0m"
        top = f"{c}┌{'─' * (w - 2)}┐{reset}"
        title_line = f"{c}│{reset} {COLORS.get('bold', '')}{str(title).center(w - 4)}{reset} {c}│{reset}"
        div = f"{c}├{'─' * (w - 2)}┤{reset}"
        bot = f"{c}└{'─' * (w - 2)}┘{reset}"
        lines = [top, title_line]
        if content:
            lines.append(div)
            if isinstance(content, list):
                for item in content:
                    item_str = str(item)
                    if len(item_str) > w - 4: item_str = item_str[:w-7] + "..."
                    lines.append(f"{c}│{reset} {item_str.ljust(w - 4)} {c}│{reset}")
            else:
                for line in str(content).splitlines():
                    if len(line) > w - 4: line = line[:w-7] + "..."
                    lines.append(f"{c}│{reset} {line.ljust(w - 4)} {c}│{reset}")
        lines.append(bot)
        res = "\n".join(lines)
        print(res)
        return res

    def table(headers, rows):
        headers = [str(h) for h in headers]
        str_rows = [[str(cell) for cell in row] for row in rows]
        col_widths = [len(h) for h in headers]
        for row in str_rows:
            for i, cell in enumerate(row):
                if i < len(col_widths):
                    col_widths[i] = max(col_widths[i], len(cell))
                else:
                    col_widths.append(len(cell))
        sep = "┌" + "┬".join("─" * (w + 2) for w in col_widths) + "┐"
        mid_sep = "├" + "┼".join("─" * (w + 2) for w in col_widths) + "┤"
        bot_sep = "└" + "┴".join("─" * (w + 2) for w in col_widths) + "┘"
        header_line = "│ " + " │ ".join(headers[i].ljust(col_widths[i]) for i in range(len(headers))) + " │"
        lines = [sep, f"\033[1;36m{header_line}\033[0m", mid_sep]
        for row in str_rows:
            row_padded = [row[i].ljust(col_widths[i]) if i < len(row) else "".ljust(col_widths[i]) for i in range(len(col_widths))]
            lines.append("│ " + " │ ".join(row_padded) + " │")
        lines.append(bot_sep)
        res = "\n".join(lines)
        print(res)
        return res

    def spinner(message="Working...", seconds=1.5):
        frames = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]
        end_time = time.time() + float(seconds)
        i = 0
        while time.time() < end_time:
            frame = frames[i % len(frames)]
            sys.stdout.write(f"\r\033[36m{frame}\033[0m {message}")
            sys.stdout.flush()
            time.sleep(0.08)
            i += 1
        sys.stdout.write(f"\r\033[32m✔\033[0m {message}\n")
        sys.stdout.flush()
        return True

    def clear():
        os.system('clear' if os.name != 'nt' else 'cls')
        return True

    return AiraModule("term", {
        "color": color,
        "rainbow": rainbow,
        "box": box,
        "table": table,
        "spinner": spinner,
        "clear": clear
    })

def create_webhook_module():
    def send(url, payload, headers=None, timeout=10.0):
        if not isinstance(payload, (dict, list, str)):
            payload = {"data": str(payload)}
        body_data = json.dumps(payload).encode('utf-8') if not isinstance(payload, str) else payload.encode('utf-8')
        req_headers = {"Content-Type": "application/json", "User-Agent": "AiraLang-Webhook/1.5.0"}
        if isinstance(headers, dict):
            req_headers.update(headers)
        req = urllib.request.Request(str(url), data=body_data, headers=req_headers, method="POST")
        try:
            with urllib.request.urlopen(req, timeout=float(timeout)) as resp:
                resp_text = resp.read().decode('utf-8')
                try:
                    return {"status": resp.status, "data": json.loads(resp_text), "ok": True}
                except:
                    return {"status": resp.status, "data": resp_text, "ok": True}
        except urllib.error.HTTPError as e:
            return {"status": e.code, "error": str(e), "ok": False}
        except Exception as e:
            return {"status": 0, "error": str(e), "ok": False}

    def listen(port, callback_handler):
        from http.server import HTTPServer, BaseHTTPRequestHandler
        class WebhookHandler(BaseHTTPRequestHandler):
            def do_POST(self):
                content_len = int(self.headers.get('Content-Length', 0))
                post_body = self.rfile.read(content_len).decode('utf-8')
                try:
                    data = json.loads(post_body)
                except:
                    data = post_body
                req_obj = {"body": data, "headers": dict(self.headers), "path": self.path}
                if hasattr(callback_handler, 'call'):
                    res = callback_handler(req_obj)
                elif callable(callback_handler):
                    res = callback_handler(req_obj)
                else:
                    res = {"received": True}
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(res if isinstance(res, (dict, list)) else {"status": "ok"}).encode())
            def log_message(self, format, *args):
                pass
        server = HTTPServer(('0.0.0.0', int(port)), WebhookHandler)
        t = threading.Thread(target=server.serve_forever, daemon=True)
        t.start()
        return {"listening": True, "port": int(port)}

    return AiraModule("webhook", {
        "send": send,
        "listen": listen
    })

def create_bot_module():
    def telegram_send(bot_token, chat_id, message, parse_mode="HTML"):
        url = f"https://api.telegram.org/bot{bot_token}/sendMessage"
        payload = {"chat_id": chat_id, "text": str(message), "parse_mode": parse_mode}
        data = json.dumps(payload).encode('utf-8')
        req = urllib.request.Request(url, data=data, headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=10) as resp:
                return json.loads(resp.read().decode('utf-8'))
        except Exception as e:
            return {"ok": False, "error": str(e)}

    def discord_send(webhook_url, content, username="Aira Bot", avatar_url=None):
        payload = {"content": str(content), "username": username}
        if avatar_url:
            payload["avatar_url"] = avatar_url
        data = json.dumps(payload).encode('utf-8')
        req = urllib.request.Request(str(webhook_url), data=data, headers={"Content-Type": "application/json", "User-Agent": "AiraLang-Bot/1.5.0"})
        try:
            with urllib.request.urlopen(req, timeout=10) as resp:
                return {"ok": True, "status": resp.status}
        except Exception as e:
            return {"ok": False, "error": str(e)}

    return AiraModule("bot", {
        "telegram_send": telegram_send,
        "telegram": telegram_send,
        "discord_send": discord_send,
        "discord": discord_send
    })

def create_env_module():
    def load(filepath=".env"):
        if not os.path.exists(filepath):
            return False
        with open(filepath, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if "=" in line:
                    k, v = line.split("=", 1)
                    k = k.strip()
                    v = v.strip().strip('"').strip("'")
                    os.environ[k] = v
        return True

    def get_env(key, default=None):
        return os.environ.get(str(key), default)

    def set_env(key, value):
        os.environ[str(key)] = str(value)
        return True

    def all_env():
        return dict(os.environ)

    return AiraModule("env", {
        "load": load,
        "get": get_env,
        "set": set_env,
        "all": all_env
    })

def create_serve_module():
    from http.server import HTTPServer, BaseHTTPRequestHandler
    routes = {}

    def route(path, handler_fn, method="GET"):
        routes[(str(method).upper(), str(path))] = handler_fn
        return True

    def get_route(path, handler_fn):
        return route(path, handler_fn, "GET")

    def post_route(path, handler_fn):
        return route(path, handler_fn, "POST")

    class AiraHttpServerHandler(BaseHTTPRequestHandler):
        def _handle(self, method):
            parsed = urllib.parse.urlparse(self.path)
            clean_path = parsed.path
            handler = routes.get((method, clean_path))
            if not handler:
                handler = routes.get(("*", clean_path))
            if not handler:
                self.send_response(404)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": "Not Found", "path": clean_path}).encode())
                return
            req_data = None
            if method in ("POST", "PUT", "PATCH"):
                length = int(self.headers.get('Content-Length', 0))
                raw_body = self.rfile.read(length).decode('utf-8', errors='ignore')
                try: req_data = json.loads(raw_body)
                except: req_data = raw_body
            query_params = dict(urllib.parse.parse_qsl(parsed.query))
            req_obj = {
                "path": clean_path,
                "method": method,
                "query": query_params,
                "body": req_data,
                "headers": dict(self.headers)
            }
            try:
                if callable(handler):
                    res = handler(req_obj)
                else:
                    res = str(handler)
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": str(e)}).encode())
                return
            self.send_response(200)
            if isinstance(res, (dict, list)):
                self.send_header('Content-Type', 'application/json')
                body_bytes = json.dumps(res).encode('utf-8')
            elif isinstance(res, str) and (res.strip().startswith("<html") or res.strip().startswith("<!DOCTYPE")):
                self.send_header('Content-Type', 'text/html')
                body_bytes = res.encode('utf-8')
            else:
                self.send_header('Content-Type', 'text/plain')
                body_bytes = str(res).encode('utf-8')
            self.end_headers()
            self.wfile.write(body_bytes)

        def do_GET(self): self._handle("GET")
        def do_POST(self): self._handle("POST")
        def do_PUT(self): self._handle("PUT")
        def do_DELETE(self): self._handle("DELETE")
        def log_message(self, format, *args): pass

    def start(port=8080, host="0.0.0.0", background=False):
        server = HTTPServer((str(host), int(port)), AiraHttpServerHandler)
        print(f"\033[32m[AiraLang Serve]\033[0m HTTP Server running at http://{host}:{port}/")
        if background:
            t = threading.Thread(target=server.serve_forever, daemon=True)
            t.start()
            return {"running": True, "port": int(port)}
        else:
            try:
                server.serve_forever()
            except KeyboardInterrupt:
                print("\n[AiraLang Serve] Server stopped.")

    return AiraModule("serve", {
        "route": route,
        "get": get_route,
        "post": post_route,
        "start": start
    })

BUILTIN_MODULES = {
    "file": create_file_module,
    "os": create_os_module,
    "math": create_math_module,
    "time": create_time_module,
    "json": create_json_module,
    "http": create_http_module,
    "crypto": create_crypto_module,
    "string": create_string_module,
    "sqlite": create_sqlite_module,
    "net": create_net_module,
    "ai": create_ai_module,
    "sec": create_sec_module,
    "thread": create_thread_module,
    "proposal": create_proposal_module,
    "android": create_android_module,
    "droidsec": create_android_module,
    "system": create_system_module,
    "hardware": create_system_module,
    "youtube": create_youtube_module,
    "yt": create_youtube_module,
    "term": create_term_module,
    "ui": create_term_module,
    "cli_ui": create_term_module,
    "webhook": create_webhook_module,
    "bot": create_bot_module,
    "telegram": create_bot_module,
    "env": create_env_module,
    "dotenv": create_env_module,
    "serve": create_serve_module,
    "server": create_serve_module
}


