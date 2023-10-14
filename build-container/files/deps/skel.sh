#!/bin/bash -xe

cat <<EOF > /etc/skel/.bash_aliases
shopt -s checkwinsize
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

alias ls='ls --color=auto'
alias ll='ls -lh'
alias la='ls -A'
alias l='ls -lAh'

alias grep='grep --color=auto'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'
EOF

cat <<EOF >> /etc/skel/.bashrc
export PATH="\$PATH:/src/soccer/build/v6/bin"
export AL_DIR=/home/nao

[ -z "\$SIMULATOR_HOST" ] && SIMULATOR_HOST=\$(getent hosts host.docker.internal | awk '{print \$1}')
[ -z "\$SIMULATOR_HOST" ] && SIMULATOR_HOST="\$(busybox ip r | grep default | awk '{print \$3}')"
export SIMULATOR_HOST
EOF

cat <<EOF >> /etc/skel/.gitconfig
[safe]
	directory = /src
	directory = /src/framework/external/cabsl
	directory = /src/framework/external/HTWKVision
	directory = /src/framework/external/entt
	directory = /src/framework/external/GSL
	directory = /src/framework/external/ELFIO
EOF
