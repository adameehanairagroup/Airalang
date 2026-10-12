# 📦 AiraLang Standard Library Modules Directory
> **Official Module Catalog for AiraLang (v1.5.0 Full-Stack Web & Cyber OSINT Edition)**  
> **Author & Creator:** Er. Adam Eehan (Founder & CEO, Aira Group of Technology) 👑

---

## 🌟 Overview
AiraLang features 22 built-in standard library modules engineered natively for full-stack web serving, AI capabilities, cyber security recon, system operations, and terminal aesthetics.

All modules are loaded natively via the `import` statement:
```aira
import "<module_name>";
```

---

## 📋 Complete Modules Catalog

| # | Module | Category | Primary Purpose | Key Methods |
|---|---|---|---|---|
| 1 | [`serve`](./serve.md) / `server` | Web Backend | Native Micro Web Server for building REST APIs | `route()`, `get()`, `post()`, `start()` |
| 2 | [`term`](./term.md) / `ui` | Terminal Aesthetics | ANSI colors, stylized boxes, ASCII data tables, braille spinners | `color()`, `rainbow()`, `box()`, `table()`, `spinner()`, `clear()` |
| 3 | [`webhook`](./webhook.md) | Event Automation | Instant JSON webhook dispatcher and background listener | `send()`, `listen()` |
| 4 | [`bot`](./bot.md) / `telegram` | Messaging Bots | Telegram Bot API and Discord webhook notifications | `telegram_send()`, `discord_send()` |
| 5 | [`env`](./env.md) / `dotenv` | Secrets & Config | `.env` file loader and environment variable manager | `load()`, `get()`, `set()`, `all()` |
| 6 | [`ai`](./ai.md) | Artificial Intelligence | Universal LLM engine (OpenAI, DeepSeek, Claude, Gemini, Groq, Ollama, OpenRouter, etc.) | `ask()`, `set_provider()`, `set_endpoint()`, `set_key()`, `providers()` |
| 7 | [`sec`](./sec.md) | Cybersecurity | Port scanning, header auditing, subdomain recon, SSL inspection, DNS lookup | `scan_ports()`, `audit_headers()`, `subdomains()`, `ssl_check()`, `dns_lookup()`, `crack_md5()` |
| 8 | [`crypto`](./crypto.md) | Security | Cryptographic hashes, JWT signing/verifying, UUIDs, Base64 | `jwt_sign()`, `jwt_verify()`, `uuid()`, `hmac()`, `sha256()`, `md5()`, `base64_encode()` |
| 9 | [`system`](./system.md) / `hardware` | PC & Hardware Telemetry | Cross-platform GPU, CPU, RAM, battery, and OS hardware telemetry | `info()`, `gpu()`, `battery()`, `ram()`, `cpu()` |
| 10 | [`android`](./android.md) / `droidsec` | Mobile Security & Recon | Android device integrity audit, root detection, and static APK security analyzer | `device_info()`, `audit_device()`, `audit_apk()`, `audit_package()`, `check_root()` |
| 11 | [`proposal`](./proposal.md) | Fun & Cyber-Prank | Automated HTTP 500 love timeout & AI sentiment checker | `ask()`, `set_key()`, `set_system()`, `wish()`, `set_wish()` |
| 12 | [`http`](./http.md) | Networking | Web requests, REST APIs, and file downloads | `get()`, `post()`, `download()`, `status()` |
| 13 | [`file`](./file.md) | System I/O | Read, write, append, and inspect local files | `read()`, `write()`, `append()`, `exists()`, `delete()` |
| 14 | [`os`](./os.md) | System | Shell commands, environment variables, CLI args, directories | `cmd()`, `env()`, `cwd()`, `listdir()`, `mkdir()`, `args()`, `is_windows()` |
| 15 | [`math`](./math.md) | Mathematics | Math functions, trigonometry, and constants | `sqrt()`, `pow()`, `abs()`, `random()`, `round()`, `pi`, `e` |
| 16 | [`time`](./time.md) | System | Epoch time, timestamps, and execution delays | `now()`, `time()`, `sleep()` |
| 17 | [`json`](./json.md) | Data | Parsing and serialization of JSON payloads | `parse()`, `stringify()`, `mkstring()` |
| 18 | [`string`](./string.md) | Utilities | Advanced text manipulation and transformation | `split()`, `replace()`, `upper()`, `lower()`, `trim()`, `join()` |
| 19 | [`sqlite`](./sqlite.md) | Database | Lightweight embedded SQL relational database | `open()`, `query()`, `execute()`, `close()` |
| 20 | [`net`](./net.md) | Networking | Host pings, TCP port checks, and IP discovery | `ping()`, `port_open()`, `local_ip()`, `fetch()` |
| 21 | [`thread`](./thread.md) | Concurrency | Thread spawning, worker pools, and parallel execution | `spawn()`, `join()`, `pool()`, `sleep()` |
| 22 | [`youtube`](./youtube.md) / `yt` | Media & Video | YouTube video/audio download engine via APM | `get()`, `stream()`, `download()`, `title`, `duration` |

---
*Maintained by the AiraLang Core Engineering Team. Created by Er. Adam Eehan.*
