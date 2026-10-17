# 🤖 AiraLang `ai` Module
> **Universal World AI & LLM Integration Engine**  
> Native multi-provider AI supporting Google Gemini, Groq, OpenAI, DeepSeek, OpenRouter, Anthropic Claude, Mistral AI, Ollama (Local/Offline), Together AI, Perplexity, Cerebras, and custom OpenAI-compatible endpoints.

## Import
```aira
import "ai";
```

## Supported World AI Providers
| Provider Key | Provider Name | Default Model | Key Prefix / Notes |
|---|---|---|---|
| `gemini` | Google Gemini | `gemini-1.5-flash` | Starts with `AIza` |
| `groq` | Groq Cloud | `openai/gpt-oss-120b` | Starts with `gsk_` |
| `openai` | OpenAI | `gpt-4o-mini` | Starts with `sk-` |
| `deepseek` | DeepSeek AI | `deepseek-chat` | Starts with `sk-` |
| `openrouter` | OpenRouter (300+ Models) | `meta-llama/llama-3.3-70b-instruct` | Starts with `sk-or-` |
| `anthropic` | Anthropic Claude | `claude-3-5-sonnet-20241022` | Starts with `sk-ant-` |
| `mistral` | Mistral AI | `mistral-small-latest` | Mistral API key |
| `ollama` | Ollama (Local / Offline) | `llama3.2` | `http://localhost:11434` (No key required) |
| `together` | Together AI | `meta-llama/Llama-3.3-70B-Instruct-Turbo` | Together API key |
| `perplexity` | Perplexity AI | `sonar` | Starts with `pplx-` |
| `cerebras` | Cerebras Fast Inference | `llama3.3-70b` | Starts with `csk-` |
| `custom` | Any Self-Hosted Server | Custom | vLLM, LM Studio, TGI, Localhost |

## Methods
- `ai.ask(prompt, [model], [key], [system], [provider])`: Unified multi-provider prompt execution.
- `ai.set_key(api_key, [provider], [model])`: Configure API key with intelligent key prefix auto-detection.
- `ai.set_provider(provider_name, [key], [model])`: Switch the active AI provider.
- `ai.set_endpoint(url, [key], [model])`: Point to ANY custom OpenAI-compatible server (vLLM, LM Studio, Ollama).
- `ai.set_model(model_name)`: Override default model.
- `ai.set_system(prompt)`: Customize the AI engine system prompt/persona (default: official Aira persona).
- `ai.providers()`: Returns a dictionary of all registered world providers.
- `ai.chat(messages, [model], [key], [provider])`: Multi-turn conversational chat.
- `ai.summarize(text, [max_words], [provider])`: Summarize text within specified word count.

### Direct Provider Shortcuts
- `ai.gemini(prompt, [model], [key])`
- `ai.groq(prompt, [model], [key])`
- `ai.openai(prompt, [model], [key])`
- `ai.deepseek(prompt, [model], [key])`
- `ai.openrouter(prompt, [model], [key])`
- `ai.anthropic(prompt, [model], [key])`
- `ai.mistral(prompt, [model], [key])`
- `ai.ollama(prompt, [model])`
- `ai.together(prompt, [model], [key])`
- `ai.perplexity(prompt, [model], [key])`
- `ai.cerebras(prompt, [model], [key])`

## Examples

### example
```aira
import "ai";

ai.set_key("YOUR_API_KEY");
ai.set_provider("PROVIDER","MODEL");

while true{
    let user_input = input("Ask to AI  : ");

    //EXIT CONTION
    if user_input == "exit" {
        say "Goood bye take care";
        break;
    }

    //ai call
    let replay = ai.ask(user_input);
    say "Aira : " + replay;
}


### 1. Smart Auto-Routing
```aira
import "ai";

# OpenRouter key automatically routes to OpenRouter
ai.set_key("sk-or-v1-YOUR_KEY");
say ai.ask("Explain quantum encryption");
```

### 2. DeepSeek AI
```aira
import "ai";

ai.set_provider("deepseek", "sk-YOUR_DEEPSEEK_KEY");
say ai.ask("Write an optimized sorting algorithm in AiraLang");
```

### 3. Local Offline AI with Ollama (Termux / PC)
```aira
import "ai";

# 100% offline, zero internet needed
ai.set_provider("ollama", model="llama3.2");
say ai.ask("What is cybersecurity?");
```

### 4. Custom Enterprise Server (vLLM / LM Studio)
```aira
import "ai";

ai.set_endpoint("http://192.168.1.50:8000/v1/chat/completions", "token", "my-custom-model");
say ai.ask("Analyze log files");
```

