#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-only
# Launcher local para RemoteSMK Studio Lab & Hardware Mapper
cd "$(dirname "$0")/web/remotesmk"
PORT=8037
echo "======================================================="
echo "  RemoteSMK Studio Lab // Hardware Mapper & Flasher"
echo "  Servidor local en: http://localhost:$PORT/"
echo "======================================================="
sleep 0.5
open "http://localhost:$PORT/"
python3 -m http.server $PORT
