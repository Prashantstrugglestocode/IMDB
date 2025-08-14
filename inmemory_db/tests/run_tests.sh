#!/usr/bin/env bash
set -euo pipefail

BIN="./build/bin/inmemory_db"
CMD="tests/commands_basic.txt"
OUT="tests/output_basic.txt"

if [ ! -x "$BIN" ]; then
  echo "binary not found: $BIN" >&2
  exit 1
fi

"$BIN" < "$CMD" > "$OUT"

pass=0; fail=0
grep -q "IMPORTED 3" "$OUT" && ((pass++)) || { echo "import count FAIL"; ((fail++)); }
grep -q "Rows: 3" "$OUT" && ((pass++)) || { echo "rows after import FAIL"; ((fail++)); }
grep -q "San Jose" "$OUT" && ((pass++)) || { echo "select where FAIL"; ((fail++)); }
grep -q "UPDATED 1" "$OUT" && ((pass++)) || { echo "update FAIL"; ((fail++)); }
grep -q "DELETED 1" "$OUT" && ((pass++)) || { echo "delete FAIL"; ((fail++)); }
grep -q "OK" "$OUT" && ((pass++)) || { echo "export OK missing"; ((fail++)); }

echo "PASS: $pass  FAIL: $fail"
if [ "$fail" -eq 0 ]; then echo "ALL TESTS PASSED"; else echo "SOME TESTS FAILED"; fi