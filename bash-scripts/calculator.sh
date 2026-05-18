#!/usr/bin/env bash

while true; do
    # read -r -p "First number: " a
    # read -r -p "Operator (+ - * /): " op
    # read -r -p "Second number: " b

    read -r -p "Expression: " a op b
    
    if [[ ! "$a" =~ ^-?[0-9]+$ ]] || [[ ! "$b" =~ ^-?[0-9]+$ ]]; then
	echo "Please enter valid integers."
	continue
    fi

    case "$op" in
	+)
	    result=$((a + b))
	    ;;
	-)
	    result=$((a - b))
	    ;;
	'*')
	    result=$((a * b))
	    ;;
	'/')
	    if ((b == 0)); then
		echo "Cannot divide by zero"
		continue
	    fi
	    result=$((a / b))
	    ;;
	*)
	    echo "Invalid operator."
	    ;;
    esac
    
    echo "Result: $result"

    read -r -p "Continue? (y/n): " again
    if [[ "$again" != "y" ]]; then
	break
    fi
done 
