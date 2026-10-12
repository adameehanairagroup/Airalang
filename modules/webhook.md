# ⚡ AiraLang Module: `webhook`

The `webhook` module enables instant event-driven webhook dispatching and lightweight background webhook receivers.

---

## 📌 Functions

### 1. `webhook.send(url, payload, headers=null, timeout=10)`
Sends an HTTP POST JSON payload to the target webhook URL.
```airalang
import "webhook";

let res = webhook.send("https://webhook.site/test", {
    "event": "alert",
    "user": "adam",
    "timestamp": "2026-10-12"
});
say f"Webhook response status: {res['status']}";
```

### 2. `webhook.listen(port, callback_handler)`
Spins up a background HTTP webhook listener. Any incoming POST request payload is parsed and passed to `callback_handler`.
```airalang
webhook.listen(5000, fn(req) {
    say f"Received webhook on {req['path']}: {req['body']}";
    return {"status": "processed"};
});
```
