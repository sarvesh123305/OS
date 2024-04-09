file_path="/tmp/checkingfile"
flag=0 ;

printTestResult(){
    local msg="$1"
    local flag="$2" 
    if [ $flag -eq 0 ]; then
        echo "$msg pass"
    else 
        echo "$msg fail"
    fi
}

rm /tmp/checkingfile
touch /tmp/checkingfile

#read100 
./testwrite  checkingfile #1.filePresent in path
if [ $? -eq 0 ]
then
    if diff -q <(echo -n "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") <(cat /tmp/checkingfile | tr -d '\n') >/dev/null; then
       :
    else
        flag=1
    fi
else
    flag = 1
fi

rm -f xyz
./testwrite xyz     #2.file not present in path
if [ $? -eq 0 ]
then
   flag=1
else
    :
fi


chmod 444 /tmp/checkingfile #3.checking with different modes
./testwrite checkingfile    
if [ $? -eq 0 ]
then
    flag=1
else
    :
fi

chmod 644 /tmp/checkingfile #reset to normal
printTestResult "read100" $flag

# end read100
flag=0;
#testread510

./testread510 $file_path
if [ $? -eq 0 ]
then   
# 1.Read bytes 5-10 from the file
bytes_read=$(dd if="$file_path" bs=1 skip=4 count=6 status=none)
    if [ "$bytes_read" == "aaaaaa" ]; then
        # echo "read510 pass"
        :
    else
        flag=1
    fi
else
    flag=1
fi

rm -f /tmp/abc # 2.filenot present
./testread510 /tmp/abc 
if [ $? -ne 0 ]
then   
        :
else
    flag=1
fi

printTestResult "testread510" $flag
#end testread510
flag=0

#write10end
./write10end $file_path #Checklast 10 bytes 
if [ $? -eq 0 ] ; then
    last10Bytes=$(tail "$file_path" | tr -d '\0' | tail -c 10)
    if [ "$last10Bytes" = "1234567890" ]; then
        :
    else
        flag=1
    fi
else 
    flag=1;
fi

chmod 444 $file_path #Check with wrong file persmissions
./write10end $file_path
if [ $? -ne 0 ] ; then
    :
else 
    flag=1;
fi
chmod 644 $file_path

printTestResult "write10end" $flag

#end write10end
flag=0

# copy
./copy $file_path /tmp/abc
if [ $? -eq 0 ]; then
    if  diff -q "$file_path" /tmp/abc > /dev/null ; then
        # echo "Success"
        :
    else 
        flag=1
    fi
fi

printTestResult "copy" $flag
#end copy
flag=0

#rename
touch /tmp/newfile
./rename /tmp/newfile /tmp/filerenamed

if [ $? -eq 0 ] 
then
    if [ -e "/tmp/filerenamed" ] && [ ! -e "/tmp/newfile" ]; then
        # echo "rename pass"
        :
    else 
        flag=1
    fi
else
        flag=1
fi


rm -f /tmp/newfile
./rename /tmp/newfile /tmp/filerenamed
if [ $? -ne 0 ] 
then
        # echo "rename pass"
        :
else
        flag=1
        # echo "rename failed"
fi

printTestResult "rename" $flag
