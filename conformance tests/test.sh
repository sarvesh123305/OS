file_path="/tmp/checkingfile"
# touch /tmp/checkingfile

# ./testwrite  checkingfile #filePresent in path
# if [ $? -eq 0 ]
# then
#     if diff -q <(echo -n "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") <(cat /tmp/checkingfile | tr -d '\n') >/dev/null; then
#         echo "read100 pass"
#     else
#         echo "read100 fail"
#     fi
# else
#     echo "read200 fail"
# fi

# ./testwrite xyz     #file not present in path
# if [ $? -eq 0 ]
# then
#    echo "read100 fail"
# else
#     echo "read100 pass"
# fi

# chmod 444 /tmp/checkingfile #checking with different modes
# ./testwrite checkingfile    
# if [ $? -eq 0 ]
# then
#     echo "read100 fail"
# else
#     echo "read100 pass"
# fi
# chmod 644 /tmp/checkingfile #reset to normal

# ./testread510 $file_path
# if [ $? -eq 0 ]
# then   

# # Read bytes 5-10 from the file
# bytes_read=$(dd if="$file_path" bs=1 skip=4 count=6 status=none)

# if [ "$bytes_read" == "aaaaaa" ]; then
#     echo "read510 pass"
# else
#     echo "read510 fail"
# fi
# else
#     echo "read510 fail"
# fi

./copy $file_path /tmp/addu
if [$? -eq 0 ] then
    if diff -q "$file_path" /tmp/addu > /dev/null; then
        echo "Success"
    else 
        echo "Failure"
    fi
fi