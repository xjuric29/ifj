#!/bin/bash

red='\033[1;31m'
green='\033[1;32m'
nc='\033[0m'

compiler="./compiler"
interpreter="tests/ic17int"
instructionFile=$(mktemp)
outputFile=$(mktemp)

echo "ICAgICAgICAgICAgICAgICBfX189PT09LV8gIF8tPT09PV9fXwogICAgICAgICAgIF8tLV5eXiMjIyMjLy8gICAgICBcXCMjIyMjXl5eLS1fCiAgICAgICAgXy1eIyMjIyMjIyMjIy8vICggICAgKSBcXCMjIyMjIyMjIyNeLV8KICAgICAgIC0jIyMjIyMjIyMjIyMvLyAgfFxeXi98ICBcXCMjIyMjIyMjIyMjIy0KICAgICBfLyMjIyMjIyMjIyMjIy8vICAgKEA6OkApICAgXFwjIyMjIyMjIyMjIyNcXwogICAgLyMjIyMjIyMjIyMjIyMoKCAgICAgXFwvLyAgICAgKSkjIyMjIyMjIyMjIyMjXAogICAtIyMjIyMjIyMjIyMjIyMjXFwgICAgKG9vKSAgICAvLyMjIyMjIyMjIyMjIyMjIy0KICAtIyMjIyMjIyMjIyMjIyMjIyNcXCAgLyBWViBcICAvLyMjIyMjIyMjIyMjIyMjIyMjLQogLSMjIyMjIyMjIyMjIyMjIyMjIyNcXC8gICAgICBcLy8jIyMjIyMjIyMjIyMjIyMjIyMjLQpfIy98IyMjIyMjIyMjIy9cIyMjIyMjKCAgIC9cICAgKSMjIyMjIy9cIyMjIyMjIyMjI3xcI18KfC8gfCMvXCMvXCMvXC8gIFwjL1wjI1wgIHwgIHwgIC8jIy9cIy8gIFwvXCMvXCMvXCN8IFx8CmAgIHwvICBWICBWICBgICAgViAgXCNcfCB8ICB8IHwvIy8gIFYgICAnICBWICBWICBcfCAgJwogICBgICAgYCAgYCAgICAgIGAgICAvIHwgfCAgfCB8IFwgICAnICAgICAgJyAgJyAgICcKICAgICAgICAgICAgICAgICAgICAoICB8IHwgIHwgfCAgKQogICAgICAgICAgICAgICAgICAgX19cIHwgfCAgfCB8IC9fXwogICAgICAgICAgICAgICAgICAodnZ2KFZWVikoVlZWKXZ2dikKCg==" | base64 -d

printf "IFJ TESTING\n\n"

for testDir in `find tests/ -maxdepth 1 -type d | sort | tail -n +2`; do
	if [ ! -f "$testDir/returnvalues" ]; then	# Pokud neni v testech return value, slozka se preskoci
		continue
	fi
	
	printf "[ == %s == ]\n" `echo "$testDir" | cut -d/ -f2` # Nazev kolekce testu

	for t in `find "$testDir" -type f -regex ".*test[0-9]*" | sort -V`; do
		testName=`echo $t | cut -d/ -f3`
		expectedReturnCode=`grep "^$testName " $testDir/returnvalues | cut -d' ' -f2`
		$compiler < "$t" > "$instructionFile" 2>/dev/null
		returnCode=$?
		
		if [ $expectedReturnCode -eq $returnCode ]; then
			if [ $expectedReturnCode -eq 0 -a ! -f "${t}.ref" ]; then
				continue	# Ocekava se, ze pokud se kod v poradku zkompiluje, bude ve slozce soubor na porovnani vysledku interpreteru
			elif [ $expectedReturnCode -eq 0 ]; then
				$interpreter "$instructionFile" > "$outputFile" 2>/dev/null
				diff "$outputFile" "${t}.ref" > /dev/null 2>&1
				if [ $? -eq 0 ]; then	# Porovnani vystupu interpreteru
					result="${green}OK${nc}"
				else
					result="${red}FAIL${nc}"
				fi
			else	# Shoduji se navratove kody compilatoru a nejsou 0
				result="${green}OK${nc}"
			fi
		else	# Neshoduji se navratove kody compilatoru
			result="${red}FAIL${nc}"
		fi

		printf "$testName $result\n"
		
		
	done
	echo
done

rm $instructionFile
rm $outputFile
