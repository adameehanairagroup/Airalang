#!/bin/bash
# =============================================================
# 🌟 AiraLang Global Native Installer for Linux & Termux
# 👑 Creator: Adam Eehan (Founder & CEO, Aira Group of Technology)
# ⚡ Engine: AiraLang S1 Duo Native Systems Flagship
# =============================================================

set -e

echo -e "\033[1;36m"
echo "     ___   _           __                    _____ __   ____             "
echo "    /   | (_)________ _/ /   ____ _____  ____ _ / ___// /  / __ \__  ______  "
echo "   / /| |/ / ___/ __ \`/ /   / __ \`/ __ \/ __ \`// __// /  / / / / / / / __ \ "
echo "  / ___ / / /  / /_/ / /___/ /_/ / / / / /_/ /___/ / /__/ /_/ / /_/ / /_/ / "
echo " /_/  |/_/_/   \\__,_/_____/\\__,_/_/ /_/\\__, //____/____/_____/\\__,_/\\____/  "
echo "                                      /____/                                "
echo -e "\033[0m"
echo -e "\033[1;32m[*] Compiling & Installing AiraLang S1 Duo Native Engine...\033[0m"

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR=""

if [ -d "/data/data/com.termux/files/usr/bin" ]; then
    BIN_DIR="/data/data/com.termux/files/usr/bin"
elif [ -d "$HOME/.local/bin" ]; then
    BIN_DIR="$HOME/.local/bin"
elif [ -d "/usr/local/bin" ] && [ -w "/usr/local/bin" ]; then
    BIN_DIR="/usr/local/bin"
else
    BIN_DIR="$HOME/bin"
    mkdir -p "$BIN_DIR"
fi

echo -e "\033[1;34m[*] Building native engine with clang...\033[0m"
cd "$REPO_DIR/s1duo"
make clean
make

cp bin/airalang "$BIN_DIR/airalang"
cp bin/airalang "$BIN_DIR/airalang3"
chmod 755 "$BIN_DIR/airalang" "$BIN_DIR/airalang3"

echo -e "\033[1;32m[+] AiraLang S1 Duo Successfully Installed to $BIN_DIR/airalang! 🚀\033[0m"
echo -e "\033[1;37m[*] Run \033[1;33mairalang\033[0m to start the REPL, or \033[1;33mairalang <script.aira>\033[0m to execute.\033[0m"
echo -e "\033[1;35m[*] Standalone ELF binary compiler: \033[1;33mairalang -b <script.aira> -o <bin>\033[0m"
