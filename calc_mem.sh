#!/bin/bash

res=2
while [ $res > 1 ]
do
	res=$(pgrep bltree)>/dev/null;
	pmap $res | grep total; sleep 5;
done
