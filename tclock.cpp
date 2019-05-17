#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <thread>
#include <atomic>
#include <chrono>
#include <signal.h>
#include <sys/ioctl.h>
#include "config.h"
#include "tclock.h"

void draw_line(int x1, int y1, int x2, int y2){
	const double rad = std::atan((y2 - y1)*1. / (x2 - x1)) + pi/2.;
	int index;
	if(rad < pi/8.){
		index = 2;
	}else if(rad < 3.*pi/8.){
		index = 3;
	}else if(rad < 5.*pi/8.){
		index = 0;
	}else if(rad < 7.*pi/8.){
		index = 1;
	}else{
		index = 2;
	}
	if(y2 > y1){
		index += 4;
	}

	const int dx = std::max(x1, x2) - std::min(x1, x2),
			dy = std::max(y1, y2) - std::min(y1, y2),
			sx = (x2 > x1)? 1: -1,
			sy = (y2 > y1)? 1: -1;

	int err = (dx > dy ? dx : -dy) / 2, e2;
	for (;;) {
		mvaddch(y1, x1, lines[index]);
		if (x1 == x2 && y1 == y2) break;
		e2 = err;
		if (e2 > -dx) { err -= dy; x1 += sx; }
		if (e2 < dy) { err += dx; y1 += sy; }
	}
}

float percent(int var, int percent){
	return double(var)/100.0*percent;
}

void tclock::make_arrows(){
	int mindim = std::min(w,h);
	H = {w/2, h/2, 
		(unsigned int)percent(mindim/2, hhperc),
		(unsigned int)percent(mindim, hwperc),
		12*60*60};
	M = {w/2, h/2, 
		(unsigned int)percent(mindim/2, mhperc),
		(unsigned int)percent(mindim, mwperc),
		60*60};
	S = {w/2, h/2, 
		(unsigned int)percent(mindim/2, shperc),
		(unsigned int)percent(mindim, swperc),
		60};
}

void tclock::update_arrow(struct arrow &a, unsigned int current){
	a.rad = pi/-2. + 2.*pi / a.max * current;
}

void tclock::draw_arrow(const struct arrow a){
	const int x1 = (int)(a.x + a.w/2.*std::cos(a.rad-pi/2.));
	const int y1 = (int)(a.y + a.w/2.*std::sin(a.rad-pi/2.));
	const int x2 = (int)(a.x + a.w/2.*std::cos(a.rad+pi/2.));
	const int y2 = (int)(a.y + a.w/2.*std::sin(a.rad+pi/2.));
	const int x3 = (int)(x1 + a.len*std::cos(a.rad));
	const int y3 = (int)(y1 + a.len*std::sin(a.rad));
	const int x4 = (int)(x2 + a.len*std::cos(a.rad));
	const int y4 = (int)(y2 + a.len*std::sin(a.rad));
	draw_line(x1, y1, x2, y2);
	draw_line(x2, y2, x4, y4);
	draw_line(x1, y1, x3, y3);
	draw_line(x3, y3, x4, y4);
}

tclock::tclock(unsigned int w, unsigned int h)
	:w(w),h(h)
{
	make_arrows();
}

tclock::tclock(const tclock &c)
	:tclock(c.get_w(), c.get_h())
{}

void tclock::update(){
	time_t now = time(0);
	tm *ltm = localtime(&now);
	const int hours = ltm->tm_hour%12,
	      mins = ltm->tm_min,
	      secs = ltm->tm_sec;
	update_arrow(H,hours*60*60);
	update_arrow(M,mins*60);
	update_arrow(S,secs);
}

void tclock::draw(){
	draw_arrows();
	draw_seconds();
}

void tclock::draw_arrows(){
	draw_arrow(H);
	draw_arrow(M);
	draw_arrow(S);
}

void tclock::draw_seconds(){
	int l = std::min(w,h);
	for(double i=0; i < 2.*pi; i += 2.*pi/30){
		draw_line(w/2 + (l/2-2)*std::cos(i),
			h/2 + (l/2-2)*std::sin(i),
			w/2+(l/2)*std::cos(i),
			h/2+(l/2)*std::sin(i));
	}
}

unsigned int tclock::get_w()const{
	return w;
}

unsigned int tclock::get_h()const{
	return h;
}

void tclock::operator=(const tclock &c){
	this->w = c.get_w();
	this->h = c.get_h();
	make_arrows();
}


std::atomic_bool end_requested, resized;

void handle_winch(int sig){
	resized = true;
}

void wait_func(){
	int key = getch();
	end_requested = true;
}

inline bool check(bool &flag, int &param, char* arg){
	if(flag){
		param = std::stoi(arg);
		flag = false;
		return true;
	}
	return false;
}

int main(int argc, char* argv[]){
	end_requested = false;
	resized = false;
	struct sigaction sa;
	sa.sa_handler = handle_winch;
	sigaction(SIGWINCH, &sa, NULL);
	int scrw, scrh;

	{
		int c;
		bool hflag = false,
		     mflag = false,
		     sflag = false,
		     charflag = false,
		     colorflag = false;
		while((c = getopt (argc, argv, "HMSw:h:C0:1:2:3:4:5:6:7:")) != -1){
			if(c == 'H'){
				hflag = true;
			}else if(c == 'M'){
				mflag = true;
			}else if(c == 'S'){
				sflag = true;
			}else if(c == 'w' || c == 'h'){
				int* param1,* param2,* param3;
				if(c == 'w'){
					param1 = &hwperc;
					param2 = &mwperc;
					param3 = &swperc;
				}else{
					param1 = &hhperc;
					param2 = &mhperc;
					param3 = &shperc;
				}
				if(!(check(hflag, *param1, optarg) ||
					check(mflag, *param2, optarg) ||
					check(sflag, *param3, optarg)))
				{
					fprintf (stderr, "Wrong usage, use option -%c with arrow literal:\n"
						"H%c - parameter for hours arrow,\n"
						"M%c - parameter for mins arrow,\n"
						"S%c - parameter for seconds arrow\n",
						c, c, c, c);
					return 1;
				}
			}else if(c == 'C'){
				charflag = true;
			}else if(c >= '0' && c <= '7'){
				if(!charflag){
					fprintf (stderr, "Wrong usage, use option -%c with quadrant literal:\n", c);
					for(int i = -1; i < 14; i+=2){
						fprintf (stderr, "-C%dc - char \'c\' for rads (%dpi/%i..%dpi/%i)\n" ,
							(i+1)/2, i, 8, i+2, 8);
					}
					return 1;
				}else{
					lines[c-'0'] = optarg[0];
				}
			}else if(c == 'c'){
				
			}else if(c == '?'){
				if (optopt == 'w' || optopt == 'h')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			}
		}
	}

	initscr();
	getmaxyx(stdscr, scrh, scrw);
	tclock cl(scrw, scrh);
	std::thread key_thr(wait_func);
	while(!end_requested){
		clear();
		if(resized){
			endwin();
			refresh();
			clear();
			getmaxyx(stdscr, scrh, scrw);
			cl = tclock(scrw,scrh);
			resized = false;
		}
		cl.update();
		cl.draw();
		refresh();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	key_thr.join();
	clear();
	endwin();
}
