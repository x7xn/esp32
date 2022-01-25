
#ifndef __LINK_H__
#define __LINK_H__

#define LINK_MOVE_TO_LAST(p) 	{while((p)->next)(p) = (p)->next;}
#define LINK_FOR_EACH(p) 		for(;NULL != (p);(p) = (p)->next)
#define LINK_ITEM_COUNT(i, n)	while(i){(n)++; (i) = (i)->next;}

#endif
