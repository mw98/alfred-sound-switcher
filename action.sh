if [[ $selected == 'false' ]]; 
then
	export PATH=/opt/homebrew/bin:/usr/local/bin:$PATH
	if [[ $CMDPATH ]]
	then 
		cmd=$CMDPATH
	else 
		cmd=SwitchAudioSource
	fi
	
	{ SwitchAudioSource -t $type -s $1 > /dev/null && message='Switched to '$1
	} || message='Error: Could not switch to '$1
	
	if [[ $NOTIFIER ]]
	then
		$NOTIFIER -group alfred-sound -title 'Sound '${(C)type} -sender com.apple.systempreferences -contentImage notif-icon-output.tiff -ignoreDnD -message $message > /dev/null
	else
		echo $message
	fi
fi