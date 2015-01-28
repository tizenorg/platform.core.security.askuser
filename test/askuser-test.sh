#!/bin/sh

if [ "$1" == "" ]; then
	echo "Script usage:"
	echo "  $0 run    - Run tests."
	echo "  $0 check  - Check if notifiaction system is operational."
	echo "  $0 resque - Try to heal notification system."
	exit 0
fi

if [ "$1" == "check" ]; then
	if [ ! -x /usr/bin/send-notification ]; then
		zypper install notification-service-test
		sleep 1
	fi
	sudo -u alice send-notification --title="MyNotification" --content="MyMessage" --icon="/usr/share/weston/terminal.png"
	echo You should see notification window now. If not, rerun the script with 'resque' argument.
	exit 0
fi

if [ "$1" == "resque" ]; then
	rm /tmp/.notification.service
	systemctl restart notifications
	systemctl restart notifications-display
	$0 test
	exit 0
fi

if [ "$1" == "run" ]; then

	cyad -b wiadro -t DENY
	cyad -s -k wiadro -c __test_client -u __test_user -p http://tizen.org/privilege/account.read -t 10
	cyad -s -c __test_client -u __test_user -p http://tizen.org/privilege/account.read -t BUCKET -m wiadro

	ERRORS=0
	SUCCESS=0

	function check {
		askuser-test-client ${1/-/} __test_client __test_session __test_user http://tizen.org/privilege/account.read > /dev/null
		RET=$?
		if [ $RET -gt 127 ]; then
			RET=$(($RET-256))
		fi

		if [ $RET -eq $1 ]; then
			echo -ne "\033[1;32m OK\033[0m"
			SUCCESS=$((SUCCESS+1))
		else
			echo -ne "\033[1;31m ERROR\033[0m"
			ERRORS=$((ERRORS+1))
		fi
	}

	echo -n Wait 60 seconds and do not press any button:
	check -1
	echo

	echo -n Press \"No:\"
	check -1
	echo

	echo -n Press \"No session:\"
	check -3
	echo

	echo -n Press \"No restart:\"
	check -3
	check -3
	echo

	systemctl restart cynara
	sleep 1

	echo -n Press \"No:\"
	check -1
	echo

	echo -n Press \"Yes:\"
	check 1
	echo

	echo -n Press \"Yes session:\"
	check 3
	echo

	echo -n Press \"Yes restart:\"
	check 3
	check 3
	echo

	systemctl restart cynara
	sleep 1

	echo -n Press \"Yes:\"
	check 1
	echo

	echo -e "Tests succeeded: \033[1;32m$SUCCESS\033[0m Tests failed: \033[1;31m$ERRORS\033[0m"

	cyad -e "" -r y -c __test_client -u __test_user -p http://tizen.org/privilege/account.read
	cyad -d wiadro

	systemctl restart cynara

fi
