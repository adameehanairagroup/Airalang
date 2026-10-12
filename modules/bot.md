# 🤖 AiraLang Module: `bot`

The `bot` module provides 1-line notification and messaging integrations for Telegram and Discord bots.

---

## 📌 Functions

### 1. `bot.telegram_send(bot_token, chat_id, message, parse_mode="HTML")`
Sends a message via the official Telegram Bot API.
```airalang
import "bot";

let token = "123456:ABC-DEF...";
let chat = "987654321";
bot.telegram_send(token, chat, "<b>AiraLang Alert:</b> Port scan finished!");
```

### 2. `bot.discord_send(webhook_url, content, username="Aira Bot", avatar_url=null)`
Dispatches an alert to a Discord channel via incoming webhook.
```airalang
bot.discord_send(
    "https://discord.com/api/webhooks/...",
    "Deployment completed on Vercel! 🚀",
    username="Aira CI/CD"
);
```
