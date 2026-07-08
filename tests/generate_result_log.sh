#!/bin/env bash

results="$(cat output.txt | gawk '
	BEGIN{
		printf "## Test Results\n"}
	{
		if($2 == "Summary:"){
			printf "|%s|%s|%s|%s|%s|%s|\n", $3, $4, $5, $6, $7, $8
			print "|-|-|-|-|-|-|"
		}
		if($1 == "suites" || $1 == "tests" || $1 == "asserts"){
			printf "|%s|%s|%s|%s|%s|%s|\n", $1, $2, $3, $4, $5, $6
		}
		if($1 == "Elapsed" && $2 == "time"){
			printf "%s %s %s %s %s\n", $1, $2, $3, $4, $5
		}
	}
')"

failed_cases="$(cat output.txt | gawk '
	BEGIN{
		printf "## Failed Test Cases\n"}
		f=0
	{
		if($1 == "Suite"){
			f=1
			printf "%s %s %s %s %s %s %s %s %s\n", $1, $2, $3, $4, $5, $6, $7, $8, $9
		}
		if($1 ~ /^[[:digit:]]+\.$/ && f = 1 ){
			f=0
			printf "%s %s %s %s %s %s\n", $1, $2, $3, $4, $5, $6
		}
	}
')"
touch results.md
printf "$results" > results.md
printf "\n" >> results.md
printf "$failed_cases" >> results.md
