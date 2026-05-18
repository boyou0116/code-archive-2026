#!/usr/bin/env bash

answer=$((RANDOM % 100 + 1))
count=0

echo "Guess a number between 1 and 100"

while ((1)); do
    read -r -p "Your guess: " guess
    if [[ ! "$guess" =~ ^[0-9]+$ ]]; then
	echo "Please enter a positive integer."
	continue
    fi

    ((count++))

    if (( guess < answer )); then
	echo "Too small."
    elif (( guess > answer )); then
	echo "Too large."
    else
	echo "Correct! You guessed it in $count tries."
	break
    fi
done


