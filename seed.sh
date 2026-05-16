#!/bin/bash
# seed.sh — populate pwdman with demo data
# Usage: ./seed.sh
#
# Make sure the server is already running before executing this script.
# Wait a moment between commands to let the server process each request.

set -e

DELAY=0.2
CLIENT="./client"

if [ ! -f "$CLIENT" ]; then
    echo "❌ client binary not found. Run 'make' first."
    exit 1
fi

echo "🌱 Seeding demo data..."
echo ""

add_entry() {
    local site="$1"
    local email="$2"
    local password="$3"
    echo "  Adding $site ($email)..."
    $CLIENT add "$site" "$email" "$password"
    sleep $DELAY
}

# --- Social
add_entry "github.com"        "john.doe@gmail.com"      "Gh!tbX92#mPq"
add_entry "gitlab.com"        "john.doe@gmail.com"      "GL@b2024#secure"
add_entry "twitter.com"       "john.doe@gmail.com"      "Tw!tt3r#X2024"
add_entry "linkedin.com"      "john.doe@gmail.com"      "L!nk3d1n#Pro9"
add_entry "reddit.com"        "johndoe95@gmail.com"     "R3dd!t#Br0wse7"

# --- Email & Productivity
add_entry "gmail.com"         "john.doe@gmail.com"      "Gm@!l#S3cur3Pass"
add_entry "outlook.com"       "johndoe@outlook.com"     "0utl00k#M@il99"
add_entry "notion.so"         "john.doe@gmail.com"      "N0t10n#W0rk$pace"
add_entry "trello.com"        "john.doe@gmail.com"      "Tr3ll0#B0@rd$2024"
add_entry "slack.com"         "john.doe@work.com"       "Sl@ck#T3am$ecure1"

# --- Dev tools
add_entry "stackoverflow.com" "john.doe@gmail.com"      "St@ckOvr#D3v2024"
add_entry "digitalocean.com"  "john.doe@gmail.com"      "D0#0ce@n$Cl0ud9"
add_entry "netlify.com"       "john.doe@gmail.com"      "N3tl!fy#D3pl0y7"
add_entry "vercel.com"        "john.doe@gmail.com"      "V3rc3l#Fr0nt3nd!"
add_entry "npmjs.com"         "johndoe_dev@gmail.com"   "Npm#Pkgs$2024X"

# --- Finance
add_entry "paypal.com"        "john.doe@gmail.com"      "P@yP@l#$ecure88"
add_entry "amazon.com"        "john.doe@gmail.com"      "Am@z0n#Sh0p$2024"
add_entry "netflix.com"       "john.doe@gmail.com"      "N3tfl!x#Str3am99"
add_entry "spotify.com"       "john.doe@gmail.com"      "Sp0t!fy#Mus!c77"
add_entry "dropbox.com"       "john.doe@gmail.com"      "Dr0pb0x#Cl0ud$12"

echo ""
echo "✅ Done! $(./client list | grep -c 'ID') entries in the database."
echo ""
echo "Try:"
echo "  ./client list"
echo "  ./client find site github"
echo "  ./client find email gmail"