# 📦 AiraLang Standard Library Modules Directory
> **Official Module Catalog for AiraLang (v1.4.4 Cross-Platform & System Edition)**  
> **Author & Creator:** Adam Eehan (Founder & CEO, Aira Group of Technology) 👑

---

## 🌟 Overview
AiraLang features 16 built-in standard library modules engineered natively for speed, AI capabilities, cyber security recon, system operations, and developer joy.

All modules are loaded natively via the `import` statement:
```aira
import "<module_name>";
```

---

## 📋 Complete Modules Catalog

| # | Module | Category | Primary Purpose | Key Methods |
|---|---|---|---|---|
| 1 | [`ai`](./ai.md) | Artificial Intelligence | Universal LLM engine (OpenAI, DeepSeek, Claude, Gemini, Groq, Ollama, OpenRouter, etc.) | `ask()`, `set_provider()`, `set_endpoint()`, `set_key()`, `providers()` |
| 2 | [`system`](./system.md) / `hardware` | PC & Hardware Telemetry | Cross-platform GPU, CPU, RAM, battery, and OS hardware telemetry | `info()`, `gpu()`, `battery()`, `ram()`, `cpu()` |
| 3 | [`android`](./android.md) / `droidsec` | Mobile Security & Recon | Android device integrity audit, root detection, and static APK security analyzer | `device_info()`, `audit_device()`, `audit_apk()`, `audit_package()`, `check_root()` |
| 4 | [`sec`](./sec.md) | Cybersecurity | Port scanning, header auditing, subdomain discovery | `scan_ports()`, `audit_headers()`, `subdomains()`, `crack_md5()` |
| 5 | [`proposal`](./proposal.md) | Fun & Cyber-Prank | Automated HTTP 500 love timeout & AI sentiment checker | `ask()`, `set_key()`, `set_system()`, `wish()`, `set_wish()` |
| 6 | [`http`](./http.md) | Networking | Web requests, REST APIs, and file downloads | `get()`, `post()`, `download()`, `status()` |
| 7 | [`file`](./file.md) | System I/O | Read, write, append, and inspect local files | `read()`, `write()`, `append()`, `exists()`, `delete()` |
| 8 | [`os`](./os.md) | System | Shell commands, environment variables, CLI args, directories | `cmd()`, `env()`, `cwd()`, `listdir()`, `mkdir()`, `args()`, `is_windows()` |
| 9 | [`math`](./math.md) | Mathematics | Math functions, trigonometry, and constants | `sqrt()`, `pow()`, `abs()`, `random()`, `round()`, `pi`, `e` |
| 10 | [`time`](./time.md) | System | Epoch time, timestamps, and execution delays | `now()`, `time()`, `sleep()` |
| 11 | [`json`](./json.md) | Data | Parsing and serialization of JSON payloads | `parse()`, `stringify()`, `mkstring()` |
| 12 | [`crypto`](./crypto.md) | Security | Cryptographic hashes and encoding utilities | `md5()`, `sha256()`, `sha1()`, `base64_encode()`, `base64_decode()` |
| 13 | [`string`](./string.md) | Utilities | Advanced text manipulation and transformation | `split()`, `replace()`, `upper()`, `lower()`, `trim()`, `join()` |
| 14 | [`sqlite`](./sqlite.md) | Database | Lightweight embedded SQL relational database | `open()`, `query()`, `execute()`, `close()` |
| 15 | [`net`](./net.md) | Networking | Host pings, TCP port checks, and IP discovery | `ping()`, `port_open()`, `local_ip()`, `fetch()` |
| 16 | [`thread`](./thread.md) | Concurrency | Thread spawning, worker pools, and parallel execution | `spawn()`, `join()`, `pool()`, `sleep()` |
| 17 | [`youtube`](./youtube.md) / `yt` | Media & Video | YouTube video/audio download engine via APM with progressive stream resolution | `get()`, `stream()`, `download()`, `title`, `duration` |

---
*Maintained by the AiraLang Core Engineering Team. Created by Adam Eehan.*
