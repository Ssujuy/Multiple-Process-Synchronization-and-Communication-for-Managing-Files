#!/bin/bash

count=1;

directory="results/*"

num_of_appearances=0

for tld in "$@"

do

    b=${#tld}
    tld_rev=""

    while :

    do
        if [ $b -ge 0 ]

        then

            tld_rev="$tld_rev${tld:$b:1}"
            b=$(($b - 1))

        else

            break

        fi
    done

    for file in $directory

    do

        while read -r line;

        do

            tmp=${line}
            length=0
            flag1=0
            rev=""

            for n in $line

            do

                if [ $flag1 -eq 0 ]

                then

                    tmp=${n}
                    length=${#n}

                fi

                flag1=$(($flag1 + 1))

                

            done

            a=$length

            while :

            do
                if [ $a -ge 0 ]

                then

                    rev="$rev${tmp:$a:1}"
                    a=$(($a - 1))

                else

                    break

                fi
            done

            flag2=1
            check=""
            length2=${#tld}

            while :

            do

                if [ $flag2 -eq $length2 ]

                then 

                    check="$check${rev:0:$flag2}"
                
                elif [ $flag2 -lt $length2 ]

                then
  
                    flag2=$(($flag2 + 1))
                    continue

                else

                    break
                
                fi

                flag2=$(($flag2 + 1))

            done

            if [ $check == $tld_rev ]

            then

                num_of_appearances=$(($num_of_appearances + 1))

            fi


        done <$file

    done

    echo "TLD : $tld has appeared : $num_of_appearances times in all .out files"

    count=$((count + 1));
    num_of_appearances=0
done