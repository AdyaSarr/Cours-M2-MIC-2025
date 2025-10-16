lw $1, 0($0)
addi $2, $0, 42
addi $3, $0, 1      
beq $1, $2, fin   
xor $3, $3, $3
fin:
sw $3, 0($0)
