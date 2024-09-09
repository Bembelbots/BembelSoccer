#!/bin/bash

emulators=(x-terminal-emulator konsole gnome-terminal urxvt256c-ml urxvt xterm)
terminal=""

for emu in "${emulators[@]}"
do
	command -v "$emu" &>/dev/null && terminal="$emu" && break
done

[ -z "$terminal" ] && echo "HELP!!! No terminal emulator found! Tried: ${emulators[*]}" && exit 1
exec "$terminal" "$@"
