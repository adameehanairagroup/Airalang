# 🎨 AiraLang Module: `term`

The `term` module provides native terminal styling, ANSI colors, ASCII bounding boxes, data tables, animated spinners, and screen controls for building high-aesthetic CLI and cyber reconnaissance tools.

---

## 📌 Functions

### 1. `term.color(color_name, text)`
Returns text formatted with the specified ANSI color.
* **Colors supported:** `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`, `bright_red`, `bright_green`, `bright_cyan`, `bold`, `dim`, `italic`, `underline`.
```airalang
import "term";
say term.color("green", "Audit Passed Successfully!");
```

### 2. `term.rainbow(text)`
Returns text with alternating bright rainbow color grading.
```airalang
say term.rainbow("AIRALANG v1.5.0 - CYBER ENGINE");
```

### 3. `term.box(title, content, color="cyan", width=50)`
Renders a stylized Unicode framed box in the terminal.
```airalang
term.box("SECURITY SCAN REPORT", [
    "Target: google.com",
    "Open Ports: 80, 443",
    "Status: SECURE"
], "cyan");
```

### 4. `term.table(headers, rows)`
Renders structured tabular data in an ASCII grid table.
```airalang
term.table(["Port", "Service", "State"], [
    ["22", "SSH", "Open"],
    ["80", "HTTP", "Open"],
    ["443", "HTTPS", "Open"]
]);
```

### 5. `term.spinner(message="Working...", seconds=1.5)`
Displays an asynchronous animated terminal braille spinner.
```airalang
term.spinner("Inspecting SSL certificate...", seconds=2);
```

### 6. `term.clear()`
Clears the active terminal screen.
