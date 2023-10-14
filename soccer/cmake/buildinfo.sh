#!/bin/bash

[ -f "$1" ] || (echo "file not found: $1"; exit 1)

OBJCOPY=${OBJCOPY:-objdump}
TMPFILE=$(mktemp)
[ -z "$GIT_USER" ] && GIT_USER="$(git config user.name)"
[ -z "$GIT_USER" ] && GIT_USER="$(id -un)"

trap 'rm -f "$TMPFILE"' EXIT

exec 3>&1 1>"$TMPFILE"

git rev-parse --abbrev-ref HEAD
git log --pretty=format:'%h' -n 1
echo
date '+%Y-%m-%d %H:%M %Z'
echo "$GIT_USER"
git status . --porcelain 2>/dev/null | grep -Ec "^(M| M)"

exec 1>&3 3>&-

$OBJCOPY --add-section .bbversion="$TMPFILE" "$1"
