query=${1:l}
echo $1
echo $query
if [[ $query != '' ]]; then
	if grep -q $query blocklist.txt; then
		sed -e "/^$query$/d" -i '' blocklist.txt
	else
		echo $query >> blocklist.txt
	fi
fi
