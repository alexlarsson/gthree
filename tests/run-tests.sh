#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_BIN="$SCRIPT_DIR/gthree-visual-tests"

# Find reference dir relative to source
REF_DIR=""
for candidate in \
    "$SCRIPT_DIR/../../tests/reference" \
    "$(dirname "$SCRIPT_DIR")/tests/reference" \
    ; do
    if [ -d "$candidate" ]; then
        REF_DIR="$(cd "$candidate" && pwd)"
        break
    fi
done

OUT_DIR="$SCRIPT_DIR/test-output"
DIFF_DIR="$SCRIPT_DIR/test-diffs"

if [ ! -x "$TEST_BIN" ]; then
    echo "Test binary not found at $TEST_BIN"
    echo "Build first with: meson compile -C <builddir>"
    exit 1
fi

# Parse arguments
MODE="compare"
while [ $# -gt 0 ]; do
    case "$1" in
        --update-ref)
            MODE="update-ref"
            shift
            ;;
        *)
            echo "Usage: $0 [--update-ref]"
            echo "  (no args)     Run tests and compare with reference"
            echo "  --update-ref  Regenerate all reference images"
            exit 1
            ;;
    esac
done

mkdir -p "$OUT_DIR"

if [ "$MODE" = "update-ref" ]; then
    [ -n "$REF_DIR" ] && mkdir -p "$REF_DIR" || { echo "Cannot find reference dir"; exit 1; }
    echo "Generating reference images..."
    echo "Output: $REF_DIR"
    echo ""
    "$TEST_BIN" --output-dir "$REF_DIR" --all 2>/dev/null
    echo ""
    echo "Reference images updated."
    exit 0
fi

# Compare mode
mkdir -p "$DIFF_DIR"
rm -f "$DIFF_DIR"/*.png

TESTS=$("$TEST_BIN" --list 2>/dev/null | grep "^  " | sed 's/^ *//')

echo "Running visual regression tests..."
echo "Binary:    $TEST_BIN"
echo "Output:    $OUT_DIR"
echo "Reference: ${REF_DIR:-(none)}"
echo "Diffs:     $DIFF_DIR"
echo ""

PASS=0
FAIL=0
NEW=0

for test in $TESTS; do
    printf "  %-30s" "$test"

    "$TEST_BIN" --output-dir "$OUT_DIR" "$test" 2>/dev/null || true

    OUT_FILE="$OUT_DIR/$test.png"
    REF_FILE="$REF_DIR/$test.png"

    if [ ! -f "$OUT_FILE" ]; then
        echo "SKIP (no output)"
        continue
    fi

    if [ -z "$REF_DIR" ] || [ ! -f "$REF_FILE" ]; then
        echo "NEW (no reference)"
        NEW=$((NEW + 1))
        continue
    fi

    if command -v compare &>/dev/null; then
        # Use fuzz factor to tolerate sub-pixel rasterization differences
        DIFF=$(compare -metric AE -fuzz 5% "$REF_FILE" "$OUT_FILE" /dev/null 2>&1 || true)
        if [ "$DIFF" = "0" ]; then
            echo "PASS"
            PASS=$((PASS + 1))
        else
            compare "$REF_FILE" "$OUT_FILE" -compose src "$DIFF_DIR/$test-delta.png" 2>/dev/null || true
            # Side-by-side: ref | new | diff
            convert "$REF_FILE" "$OUT_FILE" "$DIFF_DIR/$test-delta.png" +append "$DIFF_DIR/$test-compare.png" 2>/dev/null || true
            echo "DIFF ($DIFF px) -> $test-compare.png"
            FAIL=$((FAIL + 1))
        fi
    else
        if cmp -s "$REF_FILE" "$OUT_FILE"; then
            echo "PASS"
            PASS=$((PASS + 1))
        else
            echo "DIFF"
            FAIL=$((FAIL + 1))
        fi
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL changed, $NEW new"

if [ "$FAIL" -gt 0 ]; then
    echo ""
    echo "Delta images in: $DIFF_DIR"
    echo "To update reference: $0 --update-ref"
fi
