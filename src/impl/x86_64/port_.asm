global port_inb
global port_outb
global port_inw
global port_outw

port_inb:
	mov dx, di
	in al, dx
	movzx rax, al
	ret

port_outb:
	mov dx, di
	mov al, sil
	out dx, al
	ret

port_inw:
	mov dx, di
	in ax, dx
	movzx rax, ax
	ret

port_outw:
	mov dx, di
	mov ax, si
	out dx, ax
	ret