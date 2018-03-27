#! /bin/bash

if [ $# -ne 2 ]; then
	echo "Correct Usage: ./create_queries_xml.sh <index_file_path> <query_file_path>"
	exit
fi

echo "<parameters>"
echo "	<index>$1</index>"
echo "	<count>2000</count>"
echo "	<trecFormat>true</trecFormat>"

awk '{print "\t<query>"; print "\t\t<number>"$1"</number>"; $1 = ""; print "\t\t<text>"substr($0, 2)"</text>"; print "\t</query>"}' < $2

echo "</parameters>"
