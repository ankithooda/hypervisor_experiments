    ;; Program: exit
    ;; Executes exit syscall
    ;; No input
    ;; Output: Only the exit status
    ;;
    segment .text
    global  main

main:
    mov eax, 1                  ; 1 is the exit syscall nymber
    mov ebx, 5                  ; the status value to return
    int 0x80
