#!/bin/bash

# add user "nao" with same UID as host user
USER_ID=${LOCAL_USER_ID:-1000}

useradd --shell /bin/bash -u $USER_ID -o -c "" -M nao
mkdir /home/nao
find /etc/skel -type f -print0 | xargs -0 -I% cp % /home/nao/
chown -R nao:nao /home/nao

cd /src/soccer

if [ -z "$1" ]
then
	exec sudo -EH -u nao bash -l
else
	exec sudo -EH -u nao "$@"
fi
