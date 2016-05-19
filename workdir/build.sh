#!/bin/sh

rm -rf tl_compiler

cc ../src/scanner.c ../src/double_link.c ../src/parser.c ../src/draw_tree.c ../src/tree_op.c ../src/asm_gen.c ../src/create_cfg.c -o tl_compiler

printf "\n"
echo  "tl_compiler ELF created.."
printf "\n"
echo  "Please run exec.sh"
printf "\n"

