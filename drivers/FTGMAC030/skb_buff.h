#ifndef __SKB_BUFF_H__
#define __SKB_BUFF_H__

struct skb_buff {
	uint8_t *head;
	uint8_t *data;
	int len;
	unsigned short proto;
};

extern void init_skb(struct skb_buff *skb, uint8_t *buff, int size);
extern void skb_push(struct skb_buff *skb, int len);
extern void skb_tail(struct skb_buff *skb, int len);

#endif
