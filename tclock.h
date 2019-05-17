#pragma once
#include "config.h"


class tclock{
	struct arrow{
		unsigned int x, y;
		unsigned int len, w;
		unsigned int max;
		float rad = 0;
	}H,M,S;
	void make_arrows();
	void update_arrow(struct arrow &a, unsigned int current);
	void draw_arrow(const struct arrow a);
	void draw_arrows();
	void draw_seconds();
	unsigned int w,h;
public:
	tclock(unsigned int w, unsigned int h);
	tclock(const tclock &c);
	void update();
	void draw();
	unsigned int get_w()const;
	unsigned int get_h()const;
	void operator=(const tclock &c);
};

