#include <stdio.h>

struct exemplo_t{
	unsigned char x:3; // 3 bits para x
	unsigned char pad:5; // Ter um preenchimento para completar o byte
};


struct vmap_t{
	unsigned short vpos:4; // 4 bits para vpos
	unsigned short offset:12; // Para o offset
};

typedef struct vmap_t vmap_t;

vmap_t get_vmap(unsigned short vaddr)
{
	vmap_t vmap;

	vmap.vpos = vaddr >> 12; // Rotaciona 12 bits para a direita
	vmap.offset = vaddr << 4 >> 4; // Rotaciona 4 bits para a esquerda e depois 4 bits para a direita

	return vmap;
}


unsigned short get_raddr(vmap_t m, unsigned short rmap){ // rmap = 6 = 110
	unsigned short raddr;

	raddr = (rmap << 12) | m.offset;

	return raddr;
}

int main(int argc, char *argv[])
{
	unsigned short vaddr = 8196;

	vmap_t m = get_vmap(vaddr);
	printf("vpos: %d, offset: %d\n", m.vpos, m.offset);

	unsigned short raddr = get_raddr(m, 6);

	printf("raddr: %d\n", raddr);

	return 0;
}
