# 🛡️ AiraLang Module: `env`

The `env` module provides seamless `.env` file loading and secure environment variable access.

---

## 📌 Functions

### 1. `env.load(filepath=".env")`
Loads key-value pairs from a `.env` file into the environment. Returns `true` if loaded, `false` if file does not exist.
```airalang
import "env";

env.load(".env");
```

### 2. `env.get(key, default=null)`
Retrieves an environment variable.
```airalang
let key = env.get("GEMINI_API_KEY", "default_key");
```

### 3. `env.set(key, value)`
Sets an environment variable in the current process.
```airalang
env.set("APP_MODE", "development");
```

### 4. `env.all()`
Returns a dictionary of all current environment variables.
