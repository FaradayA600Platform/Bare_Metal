
#include "ftgmac030.h"
#include "skb_buff.h"

static void clear_buff(uint8_t *buff, int size)
{
	int i;
	for (i = 0;i < size; i++) {
		buff[i] = i & 0x00;
	}
}

void init_skb(struct skb_buff *skb, uint8_t *buff, int size)
{
	clear_buff(buff, size);
	skb->head = buff;
	skb->data = buff + size;
	skb->len = 0;
}

void skb_push(struct skb_buff *skb, int len)
{
	skb->data -= len;
	skb->len += len;
}

void skb_tail(struct skb_buff *skb, int len)
{
	skb->len -= len;
}
