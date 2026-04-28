#!/bin/bash
# Build script - Bundle and minify JS/CSS for production
# Concatenates all JS into a single file to hide individual sources
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

JS_SRC="assets/js"
CSS_SRC="assets/css"
DIST="assets/dist"

# Resolve binaries: prefer local node_modules over npx (which may not link .bin)
TERSER="node node_modules/terser/bin/terser"
CLEANCSS="node node_modules/clean-css-cli/bin/cleancss"
if [ ! -f "node_modules/terser/bin/terser" ]; then
    echo "Error: terser not installed. Run 'npm install' first." >&2
    exit 1
fi
if [ ! -f "node_modules/clean-css-cli/bin/cleancss" ]; then
    echo "Error: clean-css-cli not installed. Run 'npm install' first." >&2
    exit 1
fi

# Stage to a temp dir; only swap into $DIST on success so a partial failure
# never leaves an empty dist (which breaks the running site).
DIST_TMP="$DIST.tmp.$$"
rm -rf "$DIST_TMP"
mkdir -p "$DIST_TMP"

# Order matters: dependencies first, then consumers
JS_FILES=(
    "$JS_SRC/atsc-calculator.js"
    "$JS_SRC/cnr-tables.js"
    "$JS_SRC/ui-components.js"
    "$JS_SRC/form-handler.js"
    "$JS_SRC/backup-system.js"
    "$JS_SRC/main.js"
    "$JS_SRC/atsc-validation-system.js"
    "$JS_SRC/validation-integration.js"
)

echo "=== Bundling JavaScript ==="
# Concatenate all JS files into one
BUNDLE_TMP="$DIST_TMP/bundle.tmp.js"
> "$BUNDLE_TMP"
for file in "${JS_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "  + $(basename "$file")"
        cat "$file" >> "$BUNDLE_TMP"
        echo ";" >> "$BUNDLE_TMP"
    fi
done

# Minify the bundle (no toplevel mangle to preserve global functions called from HTML)
echo "  Minifying bundle..."
$TERSER "$BUNDLE_TMP" \
    --compress drop_debugger=true,passes=2 \
    --mangle \
    --output "$DIST_TMP/app.min.js"
rm "$BUNDLE_TMP"

echo "=== Minifying CSS ==="
# Bundle all CSS into one file
cat "$CSS_SRC"/*.css | $CLEANCSS -o "$DIST_TMP/app.min.css"
echo "  app.min.css"

# Also minify style_index.css (root level)
if [ -f "style_index.css" ]; then
    $CLEANCSS -o "$DIST_TMP/login.min.css" "style_index.css"
    echo "  login.min.css"
fi

# Atomic swap: only replace dist if all steps above succeeded
rm -rf "$DIST"
mv "$DIST_TMP" "$DIST"

echo ""
echo "=== Build complete ==="
ls -lh "$DIST"
