#!/bin/sh

if [ -z $1 ]
then
	echo "Error: run sh exec.sh <filename.tl>"
	exit 1
fi


found=0

file_name=$1
file_name=${file_name%.*}

for i in *
do
	if [ $i = "tl_compiler" ] 
	then
		found=1
		break		 
	fi
done

if [ $found -eq 0 ]
then 
	echo "Please run the build.sh to create executable"
	exit 1
else
	./tl_compiler $1 >> $file_name.tok

	if [ $?  -eq 0 ]
	then
	    printf "\n"
	    echo "Completed: created scanner output in $file_name.tok"
 	    echo "Completed: created AST.dot file in the current directory"
 	    echo "Completed: created Control_Flow.dot file in the current directory"	
 	    echo "Completed: created Assembly_Code.s file in the current directory"
	    dot -Tpng AST.dot -o $1.ast.png
	    echo "$1.ast.png file created for AST.dot file"
	    dot -Tpng Control_Flow.dot -o $1.asm.cfg.png
	    echo "$1.asm.cfg.png file created for Control_Flow.dot file"
	    printf "\n"
	    echo "NOTE: In case if your are expecting any Error messges from the TL compiler"
	    echo "      open .tok file for the Error information."
	else
	    printf "\n"
            echo "Program Compilation Failed"
	    rm -rf AST.dot
	    rm -rf Control_Flow.dot
	fi
fi


