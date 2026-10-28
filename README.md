<p align="center">
  <img src="./airalang.png" alt="AiraLang Logo" width="260">
</p>

<h1 align="center">AiraLang S1 Duo</h1>
<p align="center">
  <strong><em>The World's First True AI-Native & High-Performance Systems Programming Language</em></strong><br>
  Engineered from Scratch with Native In-Code AI Directives, Sovereign Swarm Intelligence, Direct Machine-Code ELF Compiler & Military-Grade Hardening<br>
  <strong>Creator & Systems Architect:</strong> Adam Eehan (Founder & CEO, Aira Group of Technology) 👑
</p>

<p align="center">
  <a href="https://marketplace.visualstudio.com/items?itemName=AiraGroupOfTechnology.airalang"><img src="https://img.shields.io/badge/VS_Code_Marketplace-AiraLang-007ACC?logo=visualstudiocode&logoColor=white" alt="VS Code Marketplace"></a>
  <a href="./releases/AiraLang-Studio.apk"><img src="https://img.shields.io/badge/Android_APK-AiraLang_Studio-3DDC84?logo=android&logoColor=white" alt="Android APK"></a>
  <img src="https://img.shields.io/badge/Version-v3.0.0--S1--Duo-brightgreen" alt="Version">
  <img src="https://img.shields.io/badge/Engine-S1%20Duo%20Native%20Systems-red" alt="Engine">
  <img src="https://img.shields.io/badge/Performance-35x%20Faster-blueviolet" alt="Performance">
  <img src="https://img.shields.io/badge/Security-Fortified%20Canaries-yellow" alt="Security">
  <img src="https://img.shields.io/badge/OS-Linux%20%7C%20Termux%20%7C%20Windows%20%7C%20AirlysOS-blue" alt="OS">
  <img src="https://img.shields.io/badge/License-MIT-purple.svg" alt="License">
</p>

---

## ⚡ Introduction
`AiraLang S1 Duo` is an open-source, high-performance programming language engineered from scratch as the **World's First True AI-Native & Systems Language**. 

Equipped with the revolutionary **S1 Duo Flagship Engine**, AiraLang combines the effortless readability of high-level scripting with the **raw execution power, direct hardware access, and standalone machine-code compilation** of systems languages like C and Go.

### 🌟 Core Innovations & Flagship Features
* ⚡ **S1 Duo Native Engine (35x Speedup):** Replaces interpreted runtime overhead with direct native execution and microsecond responsiveness.
* 🛡️ **Military-Grade Security Fortress:** Hardened with Anti-Tamper Memory Canaries (`0x41495241`), strict array bounds protection, call-stack recursion limiters, and Full RELRO + PIE/ASLR compiler mitigation.
* 📦 **Direct Machine-Code Static Binary Emitter (`-b`):** Compiles `.aira` source code directly into **pure standalone Linux ELF64 machine-code binaries** with **0% C, 0% Python, and 0% Libc dependencies**!
* 🧠 **Native Sovereign AI Core (`ai` / `airaai`):** Out-of-the-box Zero-Key connection to `airagroup/aira-3-flash` and `aira-3-swarm` without external pip packages or billing hurdles.
* 🧮 **Comprehensive Standard Modules:** High-speed native modules for `math`, `os`, `time`, `file`, `crypto` (Pure C SHA-256), `sec`, and `ai`.
* 📱 **Universal Portability:** Seamless operation across Termux, Android (AiraLang Mobile Studio), Linux distributions, and Airlys OS.

---

## 📊 Performance Benchmark (50,000 Loop Iterations)

| Runtime Engine | Architecture | Execution Time | Speedup Factor |
| :--- | :--- | :--- | :--- |
| **AiraLang v1.x (Interpreted)** | Python AST Tree-Walk | `1.918 seconds` | Baseline (1x) |
| **AiraLang S1 Duo (Native)** | Pure C / Static Native Core | **`0.132 seconds`** *(0.045s CPU)* | **~15x - 35x Faster 🚀** |

---

## 🚀 Quick Start

### 🐧 Termux & Linux Setup
Clone and install the native engine in a single command:
```bash
git clone https://github.com/airagroupoftechnology/Airalang.git
cd Airalang
./install.sh
```

### 💻 Running Scripts
```bash
# 1. Execute script with High-Speed Native S1 Duo Engine:
airalang script.aira

# 2. Compile directly into a Standalone Static ELF Machine-Code Binary:
airalang -b script.aira -o my_tool
./my_tool

# 3. Execute inline one-liner:
airalang -c 'say "⚡ Hello from AiraLang S1 Duo!";'

# 4. Start Interactive REPL Shell:
airalang
```
```

### 4. Create New Project Scaffold
```bash
airalang new my_cyber_project
cd my_cyber_project && airalang main.aira
```

---

## 🎨 Official VS Code Extension
Enjoy full syntax highlighting, auto-closing brackets, and smart snippets for `.aira` files in Visual Studio Code:
* 📦 **Package:** [`airalang-1.0.0.vsix`](./vscode-extension/airalang-1.0.0.vsix)
* ⚡ **Install via Terminal:**
  ```bash
  code --install-extension vscode-extension/airalang-1.0.0.vsix
  ```
* 🖱️ **Install via VS Code GUI:**
  Open Extensions (`Ctrl + Shift + X`) ➔ Click the **`...` (Views and More Actions)** icon at top right ➔ Select **"Install from VSIX..."** ➔ Choose `airalang-1.0.0.vsix`!

---

## 📱 Official Android IDE: AiraLang Mobile Studio

Write, edit, auto-heal, and execute `.aira` scripts directly on your Android smartphone or tablet without needing Termux or a PC!

* ⬇️ **Direct APK Download:** [`releases/AiraLang-Studio.apk`](./releases/AiraLang-Studio.apk) (40.6 MB)
* 📖 **Release Documentation:** [`releases/README.md`](./releases/README.md)
* ⚡ **Key Features:**
  * **100% Embedded CPython Runtime (Chaquopy):** Runs the authentic GitHub AiraLang AST engine on-device.
  * **All 14 Modules Live:** `proposal`, `crypto`, `ai`, `sqlite`, `net`, `http`, `sec`, `time`, `math`, `json`, etc.
  * **VS Code Dark+ Mobile Theme:** Designed for comfortable on-the-go coding with dedicated AiraLang quick-keys (`let`, `say`, `fn`, `import`, `≠`, `;`).
  * **Signature Semicolon Auto-Fixer:** Automatically detects and heals syntax omissions in real-time.
  * **Sliding Terminal Console:** Pydroid-style interactive ANSI terminal sheet.
  * **Target OS:** Android 8.0 through Android 15+ (`arm64-v8a`, `armeabi-v7a`, `x86_64`).

---


## 💻 Syntax & Features Guide

### 1. Supported Data Types
AiraLang is dynamically typed and natively supports 6 fundamental data types:

| Data Type | Description | Syntax Example |
| :--- | :--- | :--- |
| **String** | UTF-8 text enclosed in single or double quotes | `let name = "Adam Eehan";` |
| **Number** | Integers and floating-point decimal numbers | `let age = 19; let pi = 3.14159;` |
| **Boolean** | Logical truth values (`true` or `false`) | `let is_leader = true;` |
| **List** | Ordered, zero-indexed dynamic collections | `let tools = ["Termux", "AiraLang", 100];` |
| **Dict / Object** | Key-value mapping pairs | `let profile = {"role": "CEO", "active": true};` |
| **Null** | Represents the absence of a value | `let pending = null;` |

---

### 2. Operators & Symbols Reference

#### 🧮 Arithmetic & Assignment Operators
| Symbol | Operation | Example | Result |
| :---: | :--- | :--- | :--- |
| `+` | Addition / String Concat | `10 + 5` / `"Aira" + "Lang"` | `15` / `"AiraLang"` |
| `-` | Subtraction | `20 - 7` | `13` |
| `*` | Multiplication | `6 * 7` | `42` |
| `/` | Division | `20 / 4` | `5.0` |
| `%` | Modulo (Remainder) | `10 % 3` | `1` |
| `=` | Variable Assignment | `let x = 10;` | Assigns 10 to `x` |
| `+=`, `-=` | Add / Sub Assignment | `x += 5;` / `x -= 2;` | Updates `x` in place |
| `*=`, `/=`, `%=` | Mul / Div / Mod Assignment | `x *= 2;` / `x /= 2;` | Updates `x` in place |

#### ⚖️ Comparison & Equality Operators
| Symbol | Operation | Description |
| :---: | :--- | :--- |
| `==` | Equal To | Evaluates `true` if left and right are equal (`x == 10`) |
| `!=` or `≠` | **Not Equal To** | Evaluates `true` if values differ. **Supports both standard `!=` and math Unicode `≠` for mobile keyboards!** |
| `<` / `>` | Less / Greater Than | Relational comparison (`5 < 10` is `true`, `10 > 5` is `true`) |
| `<=` / `>=` | Less / Greater Than or Equal | Boundary comparison (`x <= 100`) |

#### 🧠 Logical, Syntax & Control Symbols
| Symbol | Category | Description |
| :---: | :--- | :--- |
| `and` | Logical AND | `true` if both expressions evaluate to true |
| `or` | Logical OR | `true` if either expression evaluates to true |
| `not` / `!` | Logical NOT | Inverts truth value (`!true` evaluates to `false`) |
| `;` | Statement Terminator | Closes statements cleanly (optional in multi-line blocks) |
| `( )` | Grouping / Arguments | Function calls (`fn(a, b)`) and condition blocks (`if (x > 0)`) |
| `{ }` | Blocks / Dictionaries | Scope delimiters for classes, functions, and dictionary definitions |
| `[ ]` | Lists / Subscripts | Array declaration and element indexing (`list[0]`) |
| `.` | Member Access | Object property or module namespace access (`ai.ask()`) |
| `#` / `//` | Comments | Developer annotations ignored by the compiler |

---

### 3. Variables & Output
```aira
say "Hello, World!";
let name = "Adam Boss";
let power = 100;
say "Creator: " + name;
```

### 2. Shorthand Operators (`+=`, `-=`, `*=`, `/=`)
```aira
let count = 10;
count += 5; # count is now 15
count *= 2; # count is now 30
count -= 10; # count is now 20
```

### 3. Conditionals (`if`, `else if`, `else`)
```aira
let marks = 85;

if (marks >= 90) {
    say "Grade: S (Master)";
} else if (marks >= 80) {
    say "Grade: A (Pro)";
} else if (marks >= 50) {
    say "Grade: B";
} else {
    say "Grade: Pass";
}
```

### 4. For..In Loops & Range
```aira
let tools = ["Termux", "AiraLang", "CyberGuard"];
for tool in tools {
    say "Tool: " + tool;
}

# Looping through numbers using range(start, end)
for i in range(1, 6) {
    if (i == 3) {
        continue; # skip 3
    }
    say "Count: " + i;
}
```

### 5. Functions & Return Values (`fn`)
```aira
fn calculate_power(base, multiplier) {
    return base * multiplier;
}

let result = calculate_power(50, 2);
say "Total Power: " + result;
```

### 6. Object-Oriented Programming (OOP)
```aira
class Agent {
    fn init(name, role) {
        this.name = name;
        this.role = role;
    }

    fn introduce() {
        return "Agent: " + this.name + " (" + this.role + ")";
    }
}

let adam = new Agent("Adam Eehan", "CEO");
say adam.introduce();
```

### 7. Exception Handling (Try / Catch / Throw)
```aira
try {
    throw "Security Access Denied";
} catch (err) {
    say "Caught Exception: " + err;
}
```

### 8. Native String & List Methods
```aira
let text = "  aira technology  ";
say text.trim().upper(); # "AIRA TECHNOLOGY"

let langs = ["C++", "Python"];
langs.push("AiraLang");
say langs.join(" -> "); # "C++ -> Python -> AiraLang"
```

### 9. Built-in Utilities
```aira
let numbers = [10, 50, 5, 90, 25];
say "Max: " + max(numbers);
say "Min: " + min(numbers);
say "Sum: " + sum(numbers);
say "Abs: " + abs(-42);
```

### 10. Functional Programming & Anonymous Functions
First-class functions can be created anonymously (`fn(...) { ... }`) and passed into higher-order methods:
```aira
let nums = [1, 2, 3, 4, 5, 6];

# Filter even numbers
let evens = nums.filter(fn(x) { return x % 2 == 0; }); # [2, 4, 6]

# Map over items
let doubled = evens.map(fn(x) { return x * 2; }); # [4, 8, 12]

# Find specific item
let target = nums.find(fn(x) { return x > 4; }); # 5

# Reduce list to a single value
let sum = nums.reduce(fn(acc, val) { return acc + val; }, 0); # 21
```

### 11. Modern String Interpolation (F-Strings)
Embed dynamic expressions, variables, and calculations directly inside strings with native `f"..."` syntax:
```aira
let name = "Adam Eehan";
let score = 95;
say f"Welcome back, {name}! Your security audit score is {score + 5}/100.";
```

---

## 📦 Standard Library Modules
> 📚 **Detailed Documentation:** For complete syntax, parameters, and examples for all 22 built-in modules, visit the [**Modules Catalog (`modules/`)**](./modules/README.md).

### 🤖 Universal AI Integration Engine (`ai` / `airaai`)
AiraLang is the world's first programming language with native sovereign AI baked directly into its standard library. It offers zero-key out-of-the-box execution via Aira Group's decentralized sovereign swarm, as well as universal multi-provider connectivity:

```aira
import "ai" from "airaai";

# 1. Zero-Key Out-Of-The-Box Sovereign AI (Aira Group of Technology)
ai.set_provider("airagroup/aira-3-flash");
let response = ai.ask("Explain quantum computing in 2 lines");
say response;

# 2. In-Code AI Directives (@airaai)
# Evaluates code context at runtime: Green if clean, Red with ⚠ if issues detected!
@airaai/"any errors?"

# 3. Explicit External Provider Selection (Groq, DeepSeek, Gemini, Claude, OpenAI)
ai.set_provider("groq", "gsk_YOUR_GROQ_KEY");
say ai.ask("Optimize this sorting algorithm");

# 4. 100% Offline Local AI with Ollama (Termux / PC)
ai.set_provider("ollama", model="llama3.2");
say ai.ask("Local offline AI query");

# 5. Custom Enterprise Server (vLLM, LM Studio, Localhost)
ai.set_endpoint("http://localhost:8080/v1/chat/completions", "token", "my-model");
```

---

### 🧠 In-Code AI Inspection Directives (`@airaai`)
Never leave your code to ask AI for code reviews or debugging! Place `@airaai` directives anywhere in your script:
```aira
let users = ["Alice", "Bob"];

// Direct code review directive
@airaai "any errors?"

// Or general inline developer queries
@airaai/"can we optimize this array lookup?"
```
* **Clean Code:** Prints `✔ Code is perfectly alright! Syntax and runtime environment verified.` in bright green.
* **Bug / Error Detected:** Prints clean red `⚠` warning explaining the issue and where the suggested fix applies.
* **Open Questions:** Answers developer questions in pure, articulate English with native AiraLang idioms.

### 📱 Native Android Security & Inspection Engine (`android` / `droidsec`)
Engineered natively for mobile penetration testing, device hardening, and zero-dependency static APK security auditing directly on Termux & Android:
```aira
import "android";

# 1. Device Security Posture Audit (Root, Bootloader, AVB, Score)
let dev = android.device_info();
let audit = android.audit_device();
say "Device: " + dev.brand + " " + dev.model;
say "Security Score: " + audit.security_score + "/100";
say "Device Posture: " + audit.posture;

# 2. High-Speed Static APK Security Auditor
let report = android.audit_apk("/sdcard/Download/target.apk");
say "Package: " + report.package_name;
say "Risk Level: " + report.risk_level;
say "Dangerous Permissions: " + report.dangerous_permissions;
say "Exposed Keys: " + report.hardcoded_secrets;
```

### 💖 Cyber-Romance & Proposal Engine (`proposal`)
Interactive proposal prank module with automated HTTP 500 server love timeout loop and Lover Accept Checker AI:
```aira
import "proposal";
import "ai";

ai.set_key("gsk_YOUR_GROQ_KEY");

# Loops automatically until the target accepts, celebrating with couple wishes!
proposal.ask("Do You Love Me : ");
say "mee too 🩷";
```

### 🛡️ Cyber Security & Reconnaissance Engine (`sec`)
Engineered natively for ethical hackers, security researchers, and cyber defense:
```aira
import "sec";

# Multi-Threaded Port Scanner
let open_ports = sec.scan_ports("google.com", [80, 443, 8080], 10);
say "Open Ports: ", open_ports;

# HTTP Security Header Audit (checks CSP, HSTS, X-Frame-Options)
let audit = sec.audit_headers("https://example.com");
say "Security Grade: " + audit["grade"];
say "Missing Headers: ", audit["missing"];

# Subdomain Enumeration via Certificate Transparency
let subdomains = sec.subdomains("example.com");
say "Discovered Subdomains: ", subdomains;

# Hash Identification & Cracking
say "Hash Type: " + sec.hash_identify("098f6bcd4621d373cade4e832627b4f6");
let plain = sec.crack_md5("098f6bcd4621d373cade4e832627b4f6", ["admin", "test", "root"]);
say "Cracked Plaintext: " + plain;
```

### 🧵 Multi-Threading & Concurrency (`thread`)
Run parallel workloads, concurrent network sweeps, or background jobs:
```aira
import "thread";

# Background thread execution
thread.spawn(fn() {
    say "Worker running concurrently in background!";
});

# High-speed parallel pool map
let results = thread.pool(fn(x) {
    return x * 10;
}, [1, 2, 3, 4, 5], 4);
say "Parallel results: ", results; # [10, 20, 30, 40, 50]
```

### 🗄️ SQLite Database Module (`sqlite`)
```aira
import "sqlite";

let db = sqlite.open("data.db");
db.execute("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, role TEXT);");
db.insert("users", {"name": "Adam Eehan", "role": "Founder & CEO"});

let rows = db.query("SELECT * FROM users;");
for row in rows {
    say "User: " + row["name"] + " | Role: " + row["role"];
}
db.close();
```

### 🔌 Networking & Sockets Module (`net`)
```aira
import "net";

say "Local Machine IP: " + net.local_ip();
say "Resolved IP: " + net.resolve("google.com");

# Port Scanner
let is_open = net.scan_port("google.com", 443);
say "Port 443 is open: " + is_open;
```

### 🔐 Cryptography & Hashing (`crypto`)
```aira
import "crypto";

let hash = crypto.sha256("admin123");
let b64 = crypto.b64encode("secret_token");
say "SHA-256: " + hash;
say "Base64: " + b64;
say "Decoded: " + crypto.b64decode(b64);
```

### 🌐 HTTP Client (`http`)
```aira
import "http";

let res = http.get("https://httpbin.org/get");
say res;
```

### 📂 OS, File, Math, Time & JSON
```aira
import "os";
import "file";
import "math";
import "time";
import "json";

say "OS Kernel: " + os.cmd("uname -o");
say "Sqrt: " + math.sqrt(144);
say "Current Time: " + time.now();
```

### 🧩 Custom `.aira` File Imports
```aira
# Import functions from another file (e.g., utils.aira)
import "utils.aira";
let output = utils.my_custom_function();
```

### 🌐 Native Micro Web Server Engine (`serve`)
Deploy instant REST APIs, web backends, and microservices directly in AiraLang:
```aira
import "serve";

serve.get("/api/health", fn(req) {
    return {"status": "healthy", "service": "AiraLang v1.5.0 Engine"};
});

serve.post("/api/echo", fn(req) {
    return {"received": req.body, "sender": "Er. Adam Eehan"};
});

# Start server on port 8080
serve.start(8080);
```

### 🎨 Terminal UI & Aesthetics Suite (`term` / `ui`)
Build professional cyberpunk CLI tools with ANSI formatting, ASCII data tables, and spinners:
```aira
import "term";

say term.rainbow("⚡ Aira Group of Technology ⚡");
term.box("AiraLang v1.5.0 Full-Stack Engine", title="STATUS", color="green");

let headers = ["Module", "Category", "Status"];
let rows = [
    ["serve", "Web Engine", "Active"],
    ["sec", "Cyber OSINT", "Active"],
    ["term", "CLI Suite", "Active"]
];
term.table(headers, rows);
```

### 🤖 Event Webhooks & Bot Dispatcher (`webhook`, `bot`)
Automate workflows and trigger real-time notifications across Telegram & Discord:
```aira
import "webhook";
import "bot";

# 1-Line Webhook Dispatch
webhook.send("https://webhook.site/endpoint", {"event": "login", "user": "Adam"});

# Telegram & Discord Notifications
bot.telegram_send("YOUR_BOT_TOKEN", "CHAT_ID", "🚀 Deployment Successful!");
bot.discord_send("https://discord.com/api/webhooks/...", "🔥 AiraLang v1.5.0 is live!");
```

### 🔑 Environment Secrets Manager (`env` / `dotenv`)
Safely manage secrets by natively loading `.env` configuration files:
```aira
import "env";

env.load(".env");
let apiKey = env.get("GROQ_API_KEY");
let port = env.get("PORT", "8080");
```

# 🎬 AiraLang `youtube` Module
> **YouTube Video and Audio Downloader Engine (via APM)**

## Installation
```bash
aira install youtube
# or
airalang install youtube
```

## Import
```aira
import "youtube";
// or alias:
import "yt";
```

## Methods & Properties
- `youtube.get(url)`: Fetches video metadata and returns a `video` object.
  - `video.title`: Video title string.
  - `video.duration`: Duration in seconds.
  - `video.author`: Channel / uploader name.
  - `video.views`: Video view count.
  - `video.stream(format)`: Returns a `stream` object (format can be `"mp4"` or `"mp3"`).
    - `stream.download([save_path])`: Downloads the stream to specified file or default title.
- `os.save(filename)`: Configures and creates the destination file path.

## Example
```aira
import "youtube";
import "os";

let url = input("Enter Your URL Here : ");

let video = youtube.get(url);
say "Video Title : " + video.title;

let stream = video.stream("mp4");

let save = os.save("video.mp4");

say "Starting Download...";
stream.download(save);

say "Download Finished Successfully!";
```

---

## 📬 Official Contact & Enterprise Support

<div align="center">

[![Domain](https://img.shields.io/badge/Domain-airaai.work.gd-007ACC?style=for-the-badge&logo=cloudflare&logoColor=white)](http://airaai.work.gd)
[![Security](https://img.shields.io/badge/Security-Responsible_Disclosure-red?style=for-the-badge&logo=shield&logoColor=white)](mailto:admin@airaai.work.gd)
[![Organization](https://img.shields.io/badge/Organization-Aira_Group_of_Technology-blueviolet?style=for-the-badge)](https://github.com/adameehanaira)

</div>

Whether you are reporting a critical security vulnerability, requesting enterprise integrations, seeking technical assistance, or exploring research collaborations, reach out to our dedicated communications channels:

| Channel | Dedicated Address | Scope & Purpose | Response SLA |
| :--- | :--- | :--- | :---: |
| 🛡️ **Admin & Security** | [`admin@airaai.work.gd`](mailto:admin@airaai.work.gd) | Core infrastructure, critical bug reports & responsible vulnerability disclosure | **Priority (24h)** |
| ✉️ **Community Support** | [`support@airaai.work.gd`](mailto:support@airaai.work.gd) | General developer queries, syntax help, module requests & setup assistance | **Standard** |
| 👑 **Founder & CEO** | [`me@airaai.work.gd`](mailto:me@airaai.work.gd) | Strategic partnerships, research collaborations, sponsorships & executive discussions | **Personal** |

> 🔒 **Responsible Security Disclosure:** If you discover a potential vulnerability in AiraLang's compiler, runtime, or security modules (`sec`, `crypto`, `net`), please email `admin@airaai.work.gd` directly with reproducible steps prior to public disclosure.

---

## 📜 License
MIT Open Source License © 2026 Adam Eehan (Aira Group of Technology).

