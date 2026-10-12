# 🌐 AiraLang Module: `serve`

The `serve` module provides a native, zero-dependency micro web server engine directly inside AiraLang. Build REST APIs, microservices, and webhook endpoints with minimal code.

---

## 📌 Functions

### 1. `serve.route(path, handler_fn, method="GET")`
Registers a route endpoint. `handler_fn` receives a request dictionary containing `path`, `method`, `query`, `body`, and `headers`.
```airalang
import "serve";

serve.route("/api/hello", fn(req) {
    return {
        "status": "ok",
        "message": "Hello from AiraLang Web Engine!",
        "client_path": req["path"]
    };
});
```

### 2. `serve.get(path, handler_fn)`
Shortcut to register a `GET` route.
```airalang
serve.get("/api/status", fn(req) {
    return {"status": "online", "uptime": "100%"};
});
```

### 3. `serve.post(path, handler_fn)`
Shortcut to register a `POST` route.
```airalang
serve.post("/api/echo", fn(req) {
    return {"received": req["body"]};
});
```

### 4. `serve.start(port=8080, host="0.0.0.0", background=false)`
Starts the HTTP server. If `background=true`, the server runs in a daemon thread so the script can continue executing.
```airalang
serve.start(port=8080);
```
