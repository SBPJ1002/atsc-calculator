#!/bin/bash
# Build script - Bundle and minify JS/CSS for production
# Concatenates all JS into a single file to hide individual sources
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

JS_SRC="assets/js"
CSS_SRC="assets/css"
DIST="assets/dist"

# Clean and create dist directory
rm -rf "$DIST"
mkdir -p "$DIST"

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
BUNDLE_TMP="$DIST/bundle.tmp.js"
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
npx terser "$BUNDLE_TMP" \
    --compress drop_debugger=true,passes=2 \
    --mangle \
    --output "$DIST/app.min.js"
rm "$BUNDLE_TMP"

echo "=== Minifying CSS ==="
# Bundle all CSS into one file
cat "$CSS_SRC"/*.css | npx cleancss -o "$DIST/app.min.css"
echo "  app.min.css"

# Also minify style_index.css (root level)
if [ -f "style_index.css" ]; then
    npx cleancss -o "$DIST/login.min.css" "style_index.css"
    echo "  login.min.css"
fi

echo ""
echo "=== Build complete ==="
ls -lh "$DIST"
